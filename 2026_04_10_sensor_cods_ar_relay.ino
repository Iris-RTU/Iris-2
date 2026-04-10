//2026.04.10 kas paveikts - ielikts galvenajā cilpā power.calibrate, i2c iekš THCO2 50KHz taktsfreq, sākuma disable PWR_USB un PWR_SPI, visa plate tērē 5.1mA
//ielikts slēdzis lai ieslēgtu relay kanālu slēgšanu.





#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <GyverPower.h>

#include <SoftwareSerial.h>
#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;

#include "Adafruit_SHT31.h"
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

SoftwareSerial radio(3, 2); // RX, TX

#include "FDC2214.h"
FDC2214 capsense(FDC2214_I2C_ADDR_0); // Use FDC2214_I2C_ADDR_1

// ************************ SENSOR SETTINGS *******************************
char groupID[] = "$THCO2";    // EC(0...9)  PH(10...19)  TH(20...29)  THCO2(30...39)  SH(40...49) aSH(40, 49)  CV(50...59)   TEST
char sensID[] = "30";       // 0  1  2  3 .... 59
long sendPeriodMinutes = 1;
bool system_with_relay = 1; //enables channel switching

float temp = 20;
float hum = 50;
float co2 = 400;
float co2read;
float voltage;
float f_ec;
uint16_t minimumSH; //absolutely not necessery, remove when convinient.
uint16_t maximumSH;


const int radioSetPin = 4;
const int voltagePin = 5;
const int sensorPowerPin = 8;
const int radioPowerPin = 7;
const int voltageReadPin = A0;
const int analogSensorPin = A1;

char sendData [50] = "";
char pData [10]  = "";


long targetSecond;
long epochSeconds;
long epochMilliseconds;
long extraSleepTime;
long sleepCorrection;

uint32_t sleepTime = 60000;

unsigned long tmr;
unsigned long tmr2;
unsigned long tmr3;
unsigned long tmr4;
unsigned long tmr5;
boolean answRec = false;
boolean dataReady = false;
unsigned long extraDelay;
int sensIDint;
byte counterr = 4;


//radio settings. Will change in programm code dynamically
 
  int radioCH[] = {100, 10, 20, 30};
  String radioCH_default = "AT+C";
  String radioCH_SET = radioCH_default + radioCH[0];
  int channel_SEL = 0;

//********************** PH ************
// #define USE_PULSE_OUT

#ifdef USE_PULSE_OUT
#include "ph_iso_surveyor.h"
Surveyor_pH_Isolated pH = Surveyor_pH_Isolated(A0);
#else
#include "ph_surveyor.h"
Surveyor_pH pH = Surveyor_pH(analogSensorPin);
#endif
int volt_avg_len = 1000;


// *********************** EC *******************
#define rx2 11
#define tx2 10
SoftwareSerial myserial(rx2, tx2);
String sensorstring = "";
boolean sensor_string_complete = false;


// *********************** SH *******************
#define CHAN_COUNT 1
float cap_min;
float cap_max;
bool calibration;
float capacitance;
int moisture;


void setup() {

  //*******POWER
  power.setSleepMode(POWERDOWN_SLEEP);     // sūta neprecīzi (+ 5-6s), tērē 20-30uA
  // power.setSleepMode(POWERSAVE_SLEEP);  // sūta neprecīzi (+ 5-6s), tērē 0.9mA

  //power.autoCalibrate();
  // power.setSleepMode(STANDBY_SLEEP);  // sūta precīzi, tērē 0.8mA
  power.calibrate(power.getMaxTimeout());
  //   power.setSleepMode(STANDBY_SLEEP);
  //  power.setSleepMode(POWERDOWN_SLEEP);
  // //   power.setSleepResolution(SLEEP_512MS);
  power.setSleepResolution(SLEEP_16MS);

power.hardwareDisable(PWR_SPI);
power.hardwareDisable(PWR_USB);


  sensIDint =  atoi(sensID);
  targetSecond = sensIDint;

  Serial.begin(250000);
  //power.setSleepMode(POWERDOWN_SLEEP);
  //int timeout = power.getMaxTimeout();

  //Serial.println(timeout);
  //power.calibrate(timeout);

  
  analogReference(INTERNAL);
  radio.begin(9600);
  radio.setTimeout(100);

  pinMode(sensorPowerPin, OUTPUT);
  digitalWrite(sensorPowerPin, LOW);
  pinMode(13, OUTPUT);
  pinMode(radioSetPin, OUTPUT);
  digitalWrite(radioSetPin, HIGH);
  pinMode(voltagePin, OUTPUT);
  pinMode(radioPowerPin, OUTPUT);
  pinMode(voltageReadPin, INPUT);

  delay(2000); // sensor restart after MC restart
  digitalWrite(sensorPowerPin, HIGH);
  delay(1000);
  Wire.begin();
  Wire.setClock(50000);
  

  // ******** SCD30 ***********
  if (strcmp(groupID, "$THCO2") == 0) {

    Serial.println("SCD30 test");
    delay(4000);
    if (airSensor.begin() == false)
    {
      Serial.println("Air sensor not detected. Please check wiring. Freezing...");
      while (1);
    }
  }

  // ******** SHT31 ***********
  if (strcmp(groupID, "$TH") == 0) {
    delay(100);
    Serial.println("SHT31 test");
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      Serial.println("Couldn't find SHT31");
      while (1) delay(1);
    }
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");
  }

  // ******** Analog PH ***********

  if (strcmp(groupID, "$PH") == 0) {
    power.hardwareDisable(PWR_I2C);
    if (pH.begin()) {                                     
      Serial.println("Loaded EEPROM");
   }
  }

  // ******** EC ***********
      Serial.println("EC test");
  myserial.begin(9600);                               //set baud rate for the software serial port to 9600
  sensorstring.reserve(30);                           //set aside some bytes for receiving data from Atlas Scientific product
  myserial.print("L,0");                              //Lai izsledz
  myserial.print('\r');                               //diodi

  delay(500);
  digitalWrite(sensorPowerPin, LOW);
  delay(100);



  // ******** SH ***********
  if (strcmp(groupID, "$SH") == 0) {
    Serial.println("Starting SH sensor");
    calibration=1;
    bool capOk = capsense.begin(0xF, 0x6, 0x5, true); //setup all four channels, autoscan with 4 channels, deglitch at 10MHz, internal oscillator
    Serial.println("SH sensor Started");
  }
    // ******** aSH ***********
    if(strcmp(groupID, "$aSH") == 0) {
      power.hardwareDisable(PWR_I2C);
minimumSH = analogRead(analogSensorPin);
 Serial.println(maximumSH);
  maximumSH = analogRead(analogSensorPin);
  Serial.println(maximumSH); //currently unneccessery as the values of analog sensor are truly analog - no inicialization required.
  delay(2000);
    }


  // ******** RADIO SETTINGS ************
  digitalWrite(radioPowerPin, HIGH);
  radio.listen();
  delay(15);
  digitalWrite(radioSetPin, LOW);

  Serial.println("Radio Settings:");
  delay(50);

  
  radio.print("AT+C100");
  delay(50);
  radio.print("AT+P8");
  delay(50);
  radio.print("AT+RX");
  delay(50);
//  while (radio.available() == 0) {
//    delay(100);
//    Serial.println("waiting radiomodule...");
//  }
  Serial.println(radio.readString());
  delay(10);
  digitalWrite(radioSetPin, HIGH);
  delay(10);
  digitalWrite(radioPowerPin, LOW);

  // ***** TEST data sending
  //  strcat(sendData, groupID);
  //  strcat(sendData, "/");
  //  strcat(sendData, sensID);
  //
  //  strcat(sendData, "/00/");
  //  itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
  //  strcat(sendData, pData);
  //  // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
  //  strcat(sendData, ";");
}



void loop() {

  

 // Serial.println("Wake!");
  if ((millis() - tmr5 > sendPeriodMinutes * 60 * 1000) && (system_with_relay)) {  // ja atbildes no RPi nav ilgāk par 3 sūtīšanas periodiem/mēģinājumiem, taisam restartu
    // wdt_enable(WDTO_15MS);
// if (false) {
  channel_SEL++;
  //check if channel_SEL is larger than actual channels
  if (channel_SEL > 4) {
    channel_SEL = 0; // better safe then sorry
    wdt_enable(WDTO_15MS); // and a reset
    delay(100);
    }

  String CHNUM = String(radioCH[channel_SEL]);
    if(radioCH[channel_SEL] < 100) {
     CHNUM = "0" + String(radioCH[channel_SEL]); 
    }

    String radioCH_SET = "";
  radioCH_SET = radioCH_default+ String(CHNUM); //output should be AT+Cxx

digitalWrite(radioPowerPin, HIGH);
  radio.listen();
  delay(30);
  digitalWrite(radioSetPin, LOW);
delay(30);
  Serial.println("Radio Settings:");
  delay(30);

Serial.println(radioCH_SET);
  // delay(15);
  radio.print(radioCH_SET);
    delay(30);
      radio.print("AT+RX");
   Serial.println(radio.readString());
  delay(30);
  digitalWrite(radioSetPin, HIGH);
  delay(30);
  digitalWrite(radioPowerPin, LOW);




  }

  if (millis() > 20000) {   // pēc sensora palaišanas nedaudz nogaidām rādījumu stabilizēšanos
    tmr3 = millis();        // fiksēsim laiku, ko konkrētais sensors patērē mērīšaanai un datu nosūtīšanai, lai atņemtu no
    //if (dataReady == true) {
    digitalWrite(radioPowerPin, HIGH);
    // Serial.println ("");
    // Serial.println ("WakeUp");
    delay(250);
    while (radio.available()) radio.read();  // laicīgi iztīra UART buferi
    radio.write(sendData);
    tmr = millis();
    Serial.print("Send data: ");
    Serial.println(sendData);


    //Serial.println("Start Waiting Answer");
    while (millis() - tmr < 1000 && answRec == false) {
      if (radio.available()) {
        tmr2 = millis();
        delay(20);
        String receivedData = radio.readStringUntil('\n'); // Nolasām datus līdz '\n'
        //   Serial.print("Ans.received: ");
        //   Serial.println(receivedData);
        int firstCommaIndex = receivedData.indexOf(',');  // Meklējam pirmo komatu, lai sadalītu datu
        if (firstCommaIndex != -1) {
          String recSensIDstr = receivedData.substring(0, firstCommaIndex); // Izdalām pirmo daļu kā sensora ID
          int recSensID = recSensIDstr.toInt();

          if (recSensID == sensIDint) {  // Salīdzinām ar mūsu sensora ID
            answRec = true;
            tmr5 = millis();
            String remainingData = receivedData.substring(firstCommaIndex + 1);    // Ja sensora ID sakrīt, turpinām ar pārējām daļām
            int secondCommaIndex = remainingData.indexOf(',');         // Meklējam otro komatu, lai sadalītu sekundes un milisekundes
            if (secondCommaIndex != -1) {
              String secondsPart = remainingData.substring(0, secondCommaIndex);    // Izdalām sekunžu un milisekunžu daļas
              String millisPart = remainingData.substring(secondCommaIndex + 1);
              // Pārvēršam uz skaitļiem
              epochSeconds = strtoul(secondsPart.c_str(), NULL, 10); // Izmantojam strtoul
              epochMilliseconds = millisPart.toInt(); // Milisekundēm int pietiek

              // Parādām rezultātu
              //            Serial.print("Answer is: RecSensor ID: ");
              //            Serial.print(recSensID);
              //            Serial.print("  Epoch Seconds: ");
              //            Serial.print(epochSeconds);
              //            Serial.print("  Epoch Milliseconds: ");
              //            Serial.println(epochMilliseconds);
            }

            // kkur šei būtu jārēķina koriģētais sleepDelay laiks un pie else  jāpieņem defaultais sūtīšanas perioda laiks - neliela korekcija apstrādei

          } else {
            // Ja sensora ID nesakrīt, iztīram buferi un ignorējam ziņojumu
            Serial.println("Sensor ID mismatch. Flushing buffer...");
            while (radio.available()) {  // Iztīram buferi, lasot visus datus no tā, līdz tas ir tukšs
              radio.read();  // Lasām nevajadzīgos datus no bufera
            }
          }power.calibrate(power.getMaxTimeout());
        }
      }
    }


    digitalWrite(radioPowerPin, LOW);  // izsledzam radiomoduli
    //}
    if (answRec != true) Serial.println("No answer from Master");
  }


   // ******************************  Power calibration ***************************************************

  power.calibrate();
  delay(50);


  // ******************************  SENSORS reading ***************************************************

  memset(sendData, 0, sizeof(sendData)); // Visu masīvu piepilda ar nullēm
  memset(pData, 0, sizeof(pData));

  // ********************** VOLTAGE reading
  analogReference(INTERNAL1V1); //should be internal
  delay(20);
  digitalWrite(voltagePin, HIGH);
  delay(20);
  voltage = analogRead(voltageReadPin);
  delay(20);
  digitalWrite(voltagePin, LOW);
  voltage =  1.1 * 43 / 10.1 *  voltage / 1023;  // Uref * R1 / R2 * ADC / 1023    33K vs 10K
   Serial.print("V:");
   Serial.println(voltage);

  // ************************************************* THCO2 (SCD30)
  if (strcmp(groupID, "$THCO2") == 0) {
    wdt_enable(WDTO_8S);    // sākam čekot uzkaršanos
    digitalWrite(sensorPowerPin, HIGH);
    pinMode(SDA, OUTPUT);
    pinMode(SCL, OUTPUT);
    digitalWrite(SDA, HIGH);
    digitalWrite(SCL, HIGH);
    Wire.begin();  // Sāk I2C komunikāciju no jauna
    Wire.setClock(50000); //pazemināts i2c atrums
    delay(10);
    // Serial.println ("Sensor starting");
    while (airSensor.begin() != true) {
      // Serial.print(".");
      delay(1);
    }
    // Serial.println ("");

    //airSensor.begin();
    counterr++;
    if (counterr > 1)  {
      delay(6000);
      counterr = 0;
    }
    //airSensor.beginMeasuring();
    // Serial.println ("Make measurments");
    while (airSensor.dataAvailable() != true) {
      //  Serial.print (".");
      delay(10);
    }
    // Serial.println ("");

    //Serial.println("Save data");
    // if (airSensor.dataAvailable()) {
    co2read = airSensor.getCO2();
    if (counterr == 0) co2 = co2read;
    temp = airSensor.getTemperature();
    hum = airSensor.getHumidity();
    Wire.end();  // Atspējo I2C
    pinMode(SDA, INPUT);
    pinMode(SCL, INPUT);
    digitalWrite(sensorPowerPin, LOW);
    wdt_disable();        // beidzam čekot uzkaršanos

    Serial.print("C:");
    Serial.print(counterr);
    Serial.print("  T:");
    Serial.print(temp);
    Serial.print("  H:");
    Serial.print(hum);
    Serial.print("  CO2:");
    Serial.print(co2);
    Serial.print("  CO2read:");
    Serial.println(co2read);


    strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(int(temp * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(hum * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(co2), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;
  }


  // ************************************************* TH(SHT31)
  if (strcmp(groupID, "$TH") == 0) {
    wdt_enable(WDTO_8S);
    digitalWrite(sensorPowerPin, HIGH);
    delay(10);
    pinMode(SDA, OUTPUT);
    pinMode(SCL, OUTPUT);
    digitalWrite(SDA, HIGH);
    digitalWrite(SCL, HIGH);
    Wire.begin();  // Sāk I2C komunikāciju no jauna
    temp = sht31.readTemperature();
    hum = sht31.readHumidity();
    Wire.end();  // Atspējo I2C
    pinMode(SDA, INPUT);
    pinMode(SCL, INPUT);
    digitalWrite(sensorPowerPin, LOW);
    wdt_disable();
    delay(100);

    strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(int(temp * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(hum * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;
  }

  // ************************************************* EC(......)

  if (strcmp(groupID, "$EC") == 0) {
    wdt_enable(WDTO_8S);
    digitalWrite(sensorPowerPin, HIGH);
    delay(2500);
    myserial.listen();
    delay(10);
    //  Serial.println("EC start");
    myserial.print("R");                                //manuali definets, lai sataisa
    myserial.print('\r');                               //single reading
    Serial.println("Call sensor");
    //    while (myserial.available() == 0) {
    //      Serial.print(".");
    //      delay(5);
    //    }
    //  Serial.println("");

    delay(500);
    while (myserial.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)myserial.read();              //get the char we just received
      sensorstring += inchar;                           //add the char to the var called sensorstring
      Serial.print(inchar);
      if (inchar == '\r') {                             //if the incoming character is a <CR>
        sensor_string_complete = true;                  //set the flag
        Serial.println("Receive Data");

      }
    }
    digitalWrite(sensorPowerPin, LOW);
    wdt_disable();
    radio.listen();

    if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
      if (isdigit(sensorstring[0]) == false) {          //if the first character in the string is a digit
        Serial.println(sensorstring);                   //send that string to the PC's serial monitor
      }
      else                                              //if the first character in the string is NOT a digit
      {
        print_EC_data();                                //then call this function
      }
      sensorstring = "";                                //clear the string
      sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
    }

    strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(int(f_ec * 10), pData, DEC);
    strcat(sendData, pData);
  
    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;


  }

  // ************************************************* PH(......)
  if (strcmp(groupID, "$PH") == 0) {
    Serial.println ("Start measuring...");
    digitalWrite(sensorPowerPin, HIGH);
    delay(4000);
    float ph = pH.read_ph();
    // Serial.print("pH=");
    // Serial.println(pH);
    digitalWrite(sensorPowerPin, LOW);

    strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(int(ph * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;
  }


  // ************************************************* SH(soil humidity)
  if (strcmp(groupID, "$SH") == 0) {
    Serial.println("Start Communication with SoilH");
    wdt_enable(WDTO_8S);
    digitalWrite(sensorPowerPin, HIGH);
    delay(100);
    Wire.begin();  // Sāk I2C komunikāciju no jauna

    bool capOk = capsense.begin(0xF, 0x6, 0x5, true);

    Serial.println("Start Measuing SoilH");
    unsigned long capa[CHAN_COUNT]; // variable to store data from FDC
    for (int i = 0; i < CHAN_COUNT; i++) { // for each channel
      // ### read 28bit data
      capa[i] = capsense.getReading28(i); //

      //************Kā Ričardam bija*****************
      float frequency = capa[i] * 0.149;
      frequency = frequency / 1000000;
      capacitance = 2 * 3.14159265359 * frequency;
      capacitance = capacitance * capacitance;
      capacitance = (10 ^ 12) / (capacitance * 0.000018);
      //***********************************************
      if (calibration == 1) {       //ja mēra pirmo reizi, tad palaižas kalibrācijas process
        cap_min = capacitance;
        cap_max = cap_min + 1;
        calibration = 0;
      }
      if (capacitance < cap_min) {
        cap_min = capacitance;
      }
      if (capacitance > cap_max) {
        cap_max = capacitance;
      }
      moisture = map(capacitance, cap_min, cap_max, 0, 100);
      String outputstring = outputstring + "Capacitance: " + String(capacitance) + " pF, Moisture: " + String(moisture) + "%";
      Serial.println(outputstring);
      outputstring = "";
    }


    Wire.end();  // Atspējo I2C
    digitalWrite(sensorPowerPin, LOW);
    wdt_disable();        // beidzam čekot uzkaršanos


    strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(int(capacitance), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(moisture), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;
  }
// *************aSH**************************
 if (strcmp(groupID, "$aSH") == 0) {
    // Serial.println("Start Communication with aSoilH");
      analogReference(DEFAULT);
      delay(10);
    wdt_enable(WDTO_8S);
        digitalWrite(sensorPowerPin, HIGH);
    delay(100);
  float moisture = 100-constrain(map(analogRead(analogSensorPin), 450, 850, 0, 100), 0, 100);
  Serial.print(moisture);
  Serial.print(" , ");
  Serial.print(analogRead(analogSensorPin));
  delay(100);
  digitalWrite(sensorPowerPin, LOW);
      delay(100);
  wdt_disable();

 strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(int(1000-analogRead(analogSensorPin)), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(moisture), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;

 }    
  // ************************************************* TEST(......)
  if (strcmp(groupID, "$TEST") == 0) {
    digitalWrite(sensorPowerPin, HIGH);
    delay(1000);
    //float pH = read_ph(read_voltage());
    // Serial.print("pH=");
    // Serial.println(pH);
    digitalWrite(sensorPowerPin, LOW);

    strcat(sendData, groupID);
    strcat(sendData, "/");
    strcat(sendData, sensID);

    strcat(sendData, "/");
    itoa(55, pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(int(voltage * 100), pData, DEC);
    strcat(sendData, pData);

    strcat(sendData, "/");
    itoa(strlen(sendData) + 2, pData, DEC); // CRC datu paketei
    strcat(sendData, pData);
    // Serial.println(strlen(sendData));   // CRC datu paketei ar pašu CRC
    strcat(sendData, ";");
    dataReady = true;
  }






  // ********************** Sleep time calculation ***************
  if (answRec == true) {
    //   sleepCorrection = sleepCorrection + targetSecond * 1000 - (epochSeconds * 1000 + epochMilliseconds);
    //  if (sleepCorrection) < 10000)  sleepCorrection = 0;

    if (targetSecond * 1000 - (epochSeconds * 1000 + epochMilliseconds) < -30000) {
      sleepCorrection = sleepCorrection + targetSecond * 1000 + (60000 - (epochSeconds * 1000 + epochMilliseconds));
    }

    else if (targetSecond * 1000 - (epochSeconds * 1000 + epochMilliseconds) > 30000) {
      sleepCorrection = sleepCorrection + (targetSecond * 1000 -60000) - (epochSeconds * 1000 + epochMilliseconds);
    }

    else (sleepCorrection = sleepCorrection + targetSecond * 1000 - (epochSeconds * 1000 + epochMilliseconds));
    //Serial.print("Correction:");
    //Serial.println(sleepCorrection);
    if (abs(sleepCorrection) > 15000) {
      sleepCorrection = 0;
    }
    Serial.print("Correction:");
    Serial.println(sleepCorrection);





    sleepTime =  (millis() - tmr2) + epochSeconds * 1000 + epochMilliseconds; // laiks ms tekošajā minūtē (sūtīšanas periodā?!)
    //    Serial.print("calcTime:");
    //    Serial.print(millis() - tmr2);
    //    Serial.print("  goSleepMom:");
    //    Serial.println(sleepTime);
    sleepTime = targetSecond * 1000 + (sendPeriodMinutes * 60 * 1000 - sleepTime) + sleepCorrection; // +250 ms  pamošanās
  }
  else {
    sleepTime = sendPeriodMinutes * 60 * 1000 - (millis() - tmr3);
  }

  if (sleepTime < 5000) sleepTime = sendPeriodMinutes * 60 * 1000 + sleepTime;
  if (sleepTime >  sendPeriodMinutes * 60 * 1000 * 2) sleepTime = sendPeriodMinutes * 20 * 1000;

  answRec = false;
  //Serial.print("Sleep Time:");
 // Serial.println(sleepTime);
  delay(10);
  //Serial.flush();
  power.sleepDelay(sleepTime);
  //Serial.flush();
  //  power.sleepDelay(sleepTime);
  // delay(sleepTime);
  //  Serial.println(millis());

  //delay(sleepTime);
  // delay būtu jākoriģē atbilstoši laikam, ko aizņem mērījuma izpilde (laiks kopš pamošanās līdz datu nosūtīšanai)
}



void print_EC_data(void) {                            //this function will pars the string

  char sensorstring_array[30];                        //we make a char array
  char *EC;                                           //char pointer used in string parsing
  char *TDS;                                          //char pointer used in string parsing
  char *SAL;                                          //char pointer used in string parsing
  char *GRAV;                                         //char pointer used in string parsing
  // float f_ec;                                         //used to hold a floating point number that is the EC

  sensorstring.toCharArray(sensorstring_array, 30);   //convert the string to a char array
  EC = strtok(sensorstring_array, ",");               //let's pars the array at each comma
  TDS = strtok(NULL, ",");                            //let's pars the array at each comma
  SAL = strtok(NULL, ",");                            //let's pars the array at each comma
  GRAV = strtok(NULL, ",");                           //let's pars the array at each comma

  //Serial.print("EC:");                                //we now print each value we parsed separately
  //Serial.println(EC);                                 //this is the EC value

  //Serial.println();                                   //this just makes the output easier to read

  f_ec = atof(EC);                                    //uncomment this line to convert the char to a float
}
