# Raspberry Pi OS 64bit ar Desktop instalācijas un konfigurācijas pamācība (Node-RED, InfluxDB, Grafana)

Pamācība, lai uzstādītu savu Raspberry PI Iris 2 projektam.

## Priekšnosacījumi

*   Ir instalēta Raspberry Pi OS 64bit versija ar darbvirsmas (desktop) vidi.
*   Ir pieeja Raspberry Pi terminālim (lokāli vai caur SSH).

## 1. Raspberry Pi konfigurācija (`raspi-config`)

Pēc OS instalēšanas veiciet sākotnējo konfigurāciju, izmantojot `sudo raspi-config` vai darbvirsmas konfigurācijas rīku (`Raspberry Pi Configuration`).

**Ieslēgt (pie `Interface Options`):**

*   `SSH:` Lai varētu pieslēgties attālināti konsolei.
*   `VNC:` Lai varētu pieslēgties attālināti desktop grafiskai videi.
*   `I2C:` Lai varētu izmantot sensorus bez radio moduļiem (nav obligāts, šajā projektā netiek izmantots).
*   `Serial Port:` Lai varētu nolasīt un rakstīt informāciju no seriālajiem piniem (šajā gadījumā informāciju caur radio moduli).
*   `Remote GPIO:` Lai kontrolētu GPIO pinus attālināti no Node-red (piemēram, automātiskai radio moduļa konfigurācijas uzstādīšanai).

**Izslēgt:**

*   `Serial console:` Lai aparatūras seriālais ports būtu pieejams datu pārsūtīšanai, nevis konsoles pieslēgumam.

## 2. Programmatūras uzstādīšana (caur SSH)

Savienojieties ar savu Raspberry Pi, izmantojot SSH (vai arī atveriet termināli lokāli).

### 2.1. Sistēmas atjaunināšana

Vispirms atjauniniet sistēmas pakotņu sarakstus un instalētās pakotnes:

```bash
sudo apt update
sudo apt upgrade -y
```

### 2.2. Node-RED instalēšana

Instalējiet Node.js un Node-RED, izmantojot oficiālo instalācijas skriptu:

```bash
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-node-red)
```

Instalācijas laikā skripts var uzdot jautājumus. Apstipriniet instalēšanu (`y`) un, ja tiek piedāvāts, instalējiet papildu Raspberry Pi specifiskos mezglus (`y`). Pamatojoties uz OCR datiem, šie iestatījumi tika konfigurēti *pēc* instalēšanas, iespējams, pirmajā palaišanas reizē caur web UI, ja tika ieslēgta lietotāja drošība:

*   **User security:** ieslēgta (var konfigurēt vēlāk Node-RED `settings.js` failā)
    *   **Username:** `admin`
    *   **Password:** `Administrator` (**Nomainiet uz drošu paroli!**)
    *   **Access:** Full Access
*   **Projects feature:** no
*   **Encryption passphrase:** `Iris2`
*   **Theme:** `Default theme`
*   **Editor:** `Monaco editor`
*   **Allow Function nodes to load external modules:** yes

Iespējojiet Node-RED automātisku startēšanos sistēmas palaišanas laikā:

```bash
sudo systemctl enable nodered.service
```

Ja Node-RED serviss vēl nedarbojas, palaidiet to:

```bash
sudo systemctl start nodered.service
```

### 2.3. InfluxDB instalēšana (v1.x)

> [!NOTE]  
> **Piezīme:** Drīzumā pamācība tiks atjaunota uz influxdb v2 versiju, kurai ir pieejama grafiskā vide.

Instalējiet InfluxDB laika sēriju datubāzi (šī pamācība izmanto v1 versiju).

1.  **Pievienojiet InfluxData repozitorija GPG atslēgu:**

    ```bash
    # InfluxData GPG atslēgas pirkstu nospiedums (informatīvi): 9D539D90D3328DC7D6C8D3B9D8FF8E1F7DF8B07E
    wget -q https://repos.influxdata.com/influxdata-archive_compat.key
    echo '393e8779c89ac8d958f81f942f9ad7fb82a25e133faddaf92e15b16e6ac9ce4c influxdata-archive_compat.key' | sha256sum -c && cat influxdata-archive_compat.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg > /dev/null
    ```

2.  **Pievienojiet InfluxDB repozitoriju:**

    ```bash
    echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list
    ```

3.  **Instalējiet InfluxDB:**

    ```bash
    sudo apt-get update && sudo apt-get install influxdb
    ```

4.  **Iespējojiet un palaidiet InfluxDB servisu:**

    ```bash
    sudo systemctl enable influxdb.service
    sudo systemctl start influxdb.service
    ```

5.  **Instalējiet InfluxDB komandrindas klientu (ja vēl nav instalēts kopā ar serveri):**

    ```bash
    sudo apt install influxdb-client
    ```

### 2.4. Grafana instalēšana

Instalējiet Grafana datu vizualizācijas platformu.

1.  **Pievienojiet Grafana GPG atslēgu un izveidojiet `keyrings` direktoriju (ja nepieciešams):**

    ```bash
    sudo mkdir -p /etc/apt/keyrings/
    wget -q -O - https://apt.grafana.com/gpg.key | gpg --dearmor | sudo tee /etc/apt/keyrings/grafana.gpg > /dev/null
    ```

2.  **Pievienojiet Grafana repozitoriju:**

    ```bash
    echo "deb [signed-by=/etc/apt/keyrings/grafana.gpg] https://apt.grafana.com stable main" | sudo tee /etc/apt/sources.list.d/grafana.list
    ```

3.  **Instalējiet Grafana:**

    ```bash
    sudo apt-get update
    sudo apt-get install -y grafana
    ```

4.  **Iespējojiet un palaidiet Grafana servisu:**

    ```bash
    sudo systemctl enable grafana-server.service
    sudo systemctl start grafana-server.service
    ```

## 3. Pārstartēšana (Ieteicams)

Lai nodrošinātu, ka visas izmaiņas stājas spēkā un servisi tiek palaisti pareizi, ir ieteicams pārstartēt Raspberry Pi:

```bash
sudo reboot
```

## 4. Piekļuves saites

Pēc pārstartēšanas varat piekļūt instalētajiem servisiem, izmantojot tīmekļa pārlūku savā datorā tajā pašā tīklā. Aizvietojiet `<RASPBERRY_PI_IP_ADRESE_VAI_NOSAUKUMS>` ar jūsu Raspberry Pi IP adresi vai hostname (noklusējuma hostname ir `raspberrypi`).

*   **Node-RED:** `http://<RASPBERRY_PI_IP_ADRESE_VAI_NOSAUKUMS>:1880/`
    *   Lietotājvārds: `admin` (vai jūsu konfigurētais)
    *   Parole: `Administrator` (vai jūsu konfigurētā)
*   **Grafana:** `http://<RASPBERRY_PI_IP_ADRESE_VAI_NOSAUKUMS>:3000/`
    *   Noklusējuma lietotājvārds: `admin`
    *   Noklusējuma parole: `admin` (pie pirmās pieslēgšanās būs jānomaina)

## 5. Papildu konfigurācija un lietošana

### 5.1. InfluxDB (v1 klients)

InfluxDB v1 datubāzi pārvalda, izmantojot `influx` komandrindas klientu.
> [!NOTE]
> **Piezīme:** InfluxDB v1 ir vecāka versija. Jaunākā ir v2 (v3 vēl ir beta versijā), kas piedāvā interaktīvu grafisko vidi.  Ja sākat jaunu projektu, apsveriet iespēju lietot InfluxDB v2 vai jaunāku.

*   **Datubāzes izveide:**
    1.  Palaidiet klientu terminālī: `influx`
    2.  Izveidojiet datubāzi (piemēram, ar nosaukumu `mydb`):
        ```sql
        CREATE DATABASE mydb
        ```
    3.  Izejiet no klienta:
        ```sql
        exit
        ```

*   **Dažas noderīgas `influx` komandas:**
    ```sql
    SHOW DATABASES       -- Parāda visas datubāzes
    USE mydb             -- Izvēlas datubāzi 'mydb' turpmākajām komandām
    SHOW MEASUREMENTS    -- Parāda visas datu tabulas (measurements) izvēlētajā datubāzē
    SHOW USERS           -- Parāda visus InfluxDB lietotājus
    SELECT * FROM "nama_tabula" LIMIT 10 -- Parāda pirmos 10 ierakstus no norādītās tabulas (ja tabulas nosaukumā ir speciālas rakstzīmes, lietojiet pēdiņas)
    ```

### 5.2. Node-RED Paletes

Lai Node-RED varētu mijiedarboties ar InfluxDB un veidot vizuālu paneli, instalējiet papildu paletes:

1.  Atveriet Node-RED savā pārlūkprogrammā (`http://<RASPBERRY_PI_IP_ADRESE_VAI_NOSAUKUMS>:1880/`).
2.  Dodieties uz izvēlni (☰ ikona augšējā labajā stūrī) -> `Manage palette`.
3.  Atveriet `Install` cilni.
4.  Meklējiet un instalējiet šādas paletes:
    *   `node-red-contrib-influxdb` (Lai sūtītu datus uz InfluxDB un nolasītu tos)
    *   `node-red-contrib-ip` (Noder IP adreses iegūšanai, ja nepieciešams)
    *   `node-red-dashboard` (Lai veidotu lietotāja saskarnes un paneļus tieši Node-RED)

### 5.3. Grafana Konfigurācija

Lai Grafana grafikus varētu iegult (embed) Node-RED dashboard vai citās vietnēs, un lai tā varētu piekļūt InfluxDB datiem, ir jāveic dažas konfigurācijas darbības.

1.  **Iegulšanas (Embedding) un Anonīmā piekļuve:**

    *   Atveriet Grafana konfigurācijas failu rediģēšanai:
        ```bash
        sudo nano /etc/grafana/grafana.ini
        ```
    *   Atrodiet `[security]` sadaļu. Noņemiet semikolu (`;`) no sākuma rindai `allow_embedding` un nomainiet vērtību uz `true`:
        ```ini
        # Set to true if you want to allow browsers to render Grafana in a <frame>, <iframe>, <embed> or <object>. Default is false.
        allow_embedding = true
        ```
    *   Atrodiet `[auth.anonymous]` sadaļu. Noņemiet semikolu (`;`) no sākuma rindai `enabled` un nomainiet vērtību uz `true` (lai atļautu anonīmu piekļuvi paneļiem):
        ```ini
        # enable anonymous access
        enabled = true
        ```
        *Ja vēlaties, lai anonīmie lietotāji varētu tikai skatīties, pārliecinieties, ka zemāk esošais `org_role` ir iestatīts uz `Viewer`.*
    *   Saglabājiet failu (`Ctrl+O`, `Enter`) un aizveriet redaktoru (`Ctrl+X`).
    *   Restartējiet Grafana servisu, lai izmaiņas stātos spēkā:
        ```bash
        sudo systemctl restart grafana-server.service
        ```

2.  **Datu avota (Data Source) izveide Grafana (ar Flux):**

   > [!IMPORTANT]
   > Ja izmanojiet InfluxDB v2, jums būs nepieciešams InfluxDB **API Token** ar atbilstošām tiesībām lasīt datus no jūsu Bucket. To var izveidot InfluxDB lietotāja saskarnē.

![image](https://github.com/user-attachments/assets/3fe3ba13-7d6f-4c9b-abd7-5384f631b6ac)

 *   Pieslēdzieties Grafana (`http://<RASPBERRY_PI_IP_ADRESE_VAI_NOSAUKUMS>:3000/`).
 *   Kreisajā izvēlnē dodieties uz `Connections` -> `Data Sources`.
 *   Noklikšķiniet `Add data source`.
 *   Izvēlieties `InfluxDB`.
 *   Konfigurējiet datu avotu:
     *   **Name:** `InfluxDB Flux` (vai cits aprakstošs nosaukums, piem., `influxdb_mydb`)
     *   **Query Language:** Izvēlieties `Flux`
     *   **HTTP -> URL:** `http://localhost:8086` (Jo Grafana un InfluxDB darbojas uz tā paša Raspberry Pi)
     *   **Auth:** Atstājiet Basic Auth, TLS Client Auth, utt., izslēgtus, ja vien nav specifiska konfigurācija. Autentifikācija notiek ar Token.
     *   **InfluxDB Details:**
         *   **Organization:** Ievadiet savu InfluxDB organizācijas nosaukumu (piemēram, `admin` vai jūsu izveidoto).
         *   **Token:** Ja izmantojiet InfluxDB v2, tad pievienojiet izveidoto API token
         *   **Default Bucket:** Ievadiet InfluxDB Bucket nosaukumu, no kura primāri nolasīsiet datus (piemēram, `mydb`).
 *   Noklikšķiniet `Save & test`. Ja viss ir pareizi (URL, organizācija, token, bucket ir atrasts), parādīsies paziņojums par veiksmīgu savienojumu (piemēram, "Datasource is working. Found X buckets").



Tagad jūsu Grafana ir konfigurēta darbam ar InfluxDB, izmantojot Flux vaicājumu valodu.

---


# Node-RED

### Node-RED izmanto, lai ievāktu datus no sensoriem, apstrādātu tos un nosūtītu uz InfluxDB datubāzi.
### Lai vizualizētu datus, tiek izmantots Node-RED Dashboard un grafana, lai ģenerētu grafikus.

> [!NOTE]
> Node-red instalācijas pamācība atrodama augstāk - [Node-RED instalēšana](#2.2.-node-red-instalēšana)

## Datu savākšana un apstrāde

![image](https://github.com/user-attachments/assets/ce3ae413-4690-46b1-8df6-7a0594f72d62)

### Datu ievākšana 
Dati tiek ievākti izmantojot `serial in node`, konfigurācija šim node redzama zemāk
![image](https://github.com/user-attachments/assets/8abd1445-baa9-4c8c-aa03-c8a386d3026d)

### Datu apstrāde
Tālāk dati tiek apstrādāti izmantojot function node ar sekojošo javascript kodu
```js
var now = new Date();
var serverTime = now.getTime();

var msg1 = {};
var msg2 = {};
var value;

//var size1 = msg.payload.length;                   // pilnais RAW ziņojuma garums
var output1 = msg.payload.split(";");               // atmetam terminatoru -> ";"

var fullMessage = output1[0];                       // lietderīgais ziņojums ar ziņojuma izmēra vērtību 

var receivedMsgSize = fullMessage.length;           //  saņemtā lietderīgā ziņojuma izmērs
msg.payload = receivedMsgSize;   // 22
//node.send(msg);

var messageParts = fullMessage.split("/");          // lietderīgā ziņojuma masīva sadalīšana pa elementiem       

var receivedMsgDataCount = messageParts.length;     // nosakām lietderīgā ziņojuma elementu skaitu (lai atrastu pēdējo kas glabā sevī ziņojuma izmēru)
msg.payload = receivedMsgDataCount;   // 6
//node.send(msg);

var sendedMsgSize = parseInt(messageParts[receivedMsgDataCount - 1]);       // nosūtītais ziņojuma izmērs, ko noteicis pats sūtītājs (jāsakrīt ar receivedMsgSize)
msg.payload = sendedMsgSize;     // 22
//node.send(msg);

if (receivedMsgSize == sendedMsgSize) {                                   // ja ziņojums saņemts pilnā apmērā, veidojam json prieks InfluxDB

var groupID = messageParts[0];                                            // nosakām sensoru grupu no kuras atnācis ziņojums


    switch (groupID) {
        case "$TH":
            msg1.measurement = "TH";
            msg.payload = "TH";
            //node.send(msg);
            // Izveidojam datu struktūru priekš InfluxDB
            msg1.payload = [
            {
                "time": Date.now(),
                "sensorID": messageParts[1],
                "temperature": parseFloat(messageParts[2]) / 100,
                "humidity": parseFloat(messageParts[3]) / 100,
                "voltage": parseFloat(messageParts[4]) / 100,
            }
            ];
            if (messageParts[2]/100 > 10) global.set("Temp1", messageParts[2]/100);
            if (messageParts[3] / 100 > 10) global.set("Hum1", messageParts[3]/100);
            break;

        case "$THCO2":
            msg1.measurement = "THCO2";
            msg.payload = "THCO2";     // 22
            // Izveidojam datu struktūru priekš InfluxDB
            msg1.payload = [
                {
                    "time": Date.now(),
                    "sensorID": messageParts[1],
                    "temperature": parseFloat(messageParts[2]) / 100,
                    "humidity": parseFloat(messageParts[3]) / 100,
                    "co2": parseFloat(messageParts[4]),
                    "voltage": parseFloat(messageParts[5]) / 100,
                }
                 ];
            if (messageParts[2] / 100 > 10) global.set("Temp2", messageParts[2]/100);
            if (messageParts[3] / 100 > 10) global.set("Hum2", messageParts[3]/100);
            if (messageParts[4] > 410) global.set("Co22", messageParts[4]);
            break; 

        case "$EC":
            msg1.measurement = "EC";
            msg.payload = "EC";     // 22
            // Izveidojam datu struktūru priekš InfluxDB
            msg1.payload = [
                {
                    "time": Date.now(),
                    "sensorID": messageParts[1],
                    "EC": parseFloat(messageParts[2]) / 10,
                    "voltage": parseFloat(messageParts[3]) / 100,
                }
            ];
            global.set("EC3", messageParts[2] / 10);
            break;

        case "$PH":
            msg1.measurement = "PH";
            msg.payload = "PH";     // 22
            // Izveidojam datu struktūru priekš InfluxDB
            msg1.payload = [
                {
                    "time": Date.now(),
                    "sensorID": messageParts[1],
                    "PH": parseFloat(messageParts[2]) / 100,
                    "voltage": parseFloat(messageParts[3]) / 100,
                }
            ];
            if (messageParts[2] / 100 > 3)  global.set("PH4", messageParts[2] / 100);
            break;
    }
    let noww = new Date();
    let seconds = noww.getSeconds();    // Tekošās sekundes
    let milliseconds = noww.getMilliseconds(); // Tekošās milisekundes
    msg2.payload = `${messageParts[1]},${seconds},${milliseconds}`; // Formatē kā 'sekundes,milisekundes'
    return [msg1, msg2];
}
else {
    msg.payload = "The message size is incorrect";     
    node.send(msg);
}
```

### Datu sūtīšana
Pirms datu sūtīšanas tiek nosūtīta atbilde caur radio moduli atpakaļ izmantojot serial out node ar iepriekš izveidoto serial konfigurāciju.
Tālāk pārstrādātā informācija tiek nosūtīta uz InfluxDB izmantojot InfluxDB out node.

> [!NOTE]
> Vispirms jāsakonfigurē InfluxDB attiecīgajai versijai un datubāzei

![image](https://github.com/user-attachments/assets/3326c443-49e7-45d1-970f-4518bd694f0f)

## Datu vizualizācija
Datu vizualizācija tiek īstenota izmantojot Node-RED Dashboard
