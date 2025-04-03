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

2.  **Datu avota (Data Source) izveide Grafana:**

> [!Caution]
> TODO: jāpārveido data source pamācība uz flux query valodu.

    *   Pieslēdzieties Grafana (`http://<RASPBERRY_PI_IP_ADRESE_VAI_NOSAUKUMS>:3000/`).
    *   Kreisajā izvēlnē dodieties uz `Connections` -> `Data Sources`.
    *   Noklikšķiniet `Add data source`.
    *   Izvēlieties `InfluxDB`.
    *   Konfigurējiet datu avotu (pamatojoties uz OCR attēlu un standarta iestatījumiem):
        *   **Name:** `influxdb mydb` (vai cits aprakstošs nosaukums)
        *   **Query Language:** `InfluxQL`
        *   **HTTP -> URL:** `http://localhost:8086` (Jo Grafana un InfluxDB darbojas uz tā paša Raspberry Pi)
        *   **Auth:** Atstājiet noklusējuma iestatījumus (piem., Basic Auth izslēgtu), ja InfluxDB nav konfigurēta autentifikācija. Ja esat izveidojis InfluxDB lietotāju/paroli, ievadiet tos šeit.
        *   **InfluxDB Details -> Database:** `mydb` (Ievadiet jūsu izveidotās datubāzes nosaukumu)
        *   **InfluxDB Details -> User / Password:** Atstājiet tukšu, ja InfluxDB datubāzei `mydb` nav specifiska lietotāja/paroles.
        *   **HTTP Method:** `GET` (parasti piemērots InfluxQL)
    *   Noklikšķiniet `Save & test`. Ja viss ir pareizi, parādīsies paziņojums par veiksmīgu savienojumu ("Data source is working").
