#define uchar unsigned char
#define uint  unsigned int

// Pin definīcijas
const int latchPin = A3;  // Latch pin 74HC595
const int clockPin = A4;  // Clock pin 74HC595
const int dataPin = A5;   // Data pin 74HC595
const int OE_595 = A2;    // Enable pin 74HC595

#define MODULE_ID 100       // Moduļa unikālais ID
byte relayStat[8] = {0};  // Masīvs releju stāvokļiem
uchar relay_port = 0;     // Releju stāvokļus glabājošs mainīgais

#include <SoftwareSerial.h>
SoftwareSerial radio(3, 2); // RX, TX
const int radioSetPin = 4;




void TubeDisplayOneBit() {
  uchar relay_dat = relay_port;

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, relay_dat);
  digitalWrite(latchPin, HIGH);
}

void updateRelayState() {
  relay_port = 0; // Nodzēš visus relejus
  for (int i = 0; i < 8; i++) {
    bitWrite(relay_port, i, relayStat[i]); // Iestata attiecīgo biti
  }
  TubeDisplayOneBit(); // Atjaunina displeju
}

void readSerialData() {
  if (radio.available()) {
    String receivedData = radio.readStringUntil('\n');

    // Sadala datus pēc "/"
    String values[9]; // 1 ID + 8 releju stāvokļi
    int startIndex = 0;
    int sepIndex = 0;
    int valueCount = 0;

    while ((sepIndex = receivedData.indexOf('/', startIndex)) != -1 && valueCount < 8) {
      values[valueCount++] = receivedData.substring(startIndex, sepIndex);
      startIndex = sepIndex + 1;
    }

    // Pēdējā vērtība (pēc pēdējās "/")
    values[valueCount++] = receivedData.substring(startIndex);

    // Pārbaude – jābūt tieši 9 vērtībām
    if (valueCount != 9) {
      Serial.println("Ignored: Incorrect number of values");
      return;
    }

    // Pārbaude, vai pirmais lauks ir korekts ID
    int receivedID = values[0].toInt();
    if (String(receivedID) != values[0]) {
      Serial.println("Ignored: Invalid ID format");
      return;
    }

    if (receivedID != MODULE_ID) {
      Serial.println("Ignored: Wrong ID");
      return;
    }

    // Nolasa releju stāvokļus
    for (int i = 0; i < 8; i++) {
      relayStat[i] = values[i + 1].toInt();
    }

    updateRelayState();
    Serial.println("Relay state updated");
  }
}

void setup() {
  Serial.begin(250000); // Sāk seriālo sakaru

  radio.begin(9600); // Sāk SoftwareSerial sakaru
  radio.setTimeout(100);
  pinMode(radioSetPin, OUTPUT);
  delay(500);
  digitalWrite(radioSetPin, LOW);

  Serial.println("Radio Settings:");
  delay(500);
  radio.print("AT+C100");
  delay(100);
  radio.print("AT+P8");
  delay(100);
  radio.print("AT+RX");
  delay(100);
  while (radio.available() == 0)
    delay(100);
  Serial.println(radio.readString());
  delay(100);
  digitalWrite(radioSetPin, HIGH);




  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(OE_595, OUTPUT);

  digitalWrite(OE_595, LOW); // Iespējo 74HC595
  updateRelayState(); // Sāk ar izslēgtiem relejiem
}

void loop() {
  readSerialData(); // Klausās seriālo portu un atjaunina relejus
}
