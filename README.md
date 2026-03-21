# KabelMarker Pro  
**Fast cable marker controller on ESP8266 with on‑device web UI**  
*No external app required — just connect to the AP and press **MARK***  

> **Ownership** – Embedded in source & assets: **#Amir Mobasheraghdam**

---

## 🇬🇧 English

### Features
- Standalone Wi‑Fi Access Point (open) for instant access  
- Large, mobile‑first UI (iPhone‑ready)  
- Fast servo motion with adaptive stepping  
- Audible confirmation tones on completion  
- Physical trigger via D7–GND (button or relay)  

### Hardware
| Component | Pin |
|-----------|-----|
| Servo signal | D5 |
| Buzzer | D6 |
| Status LED | Built‑in |
| External trigger (NO to GND) | D7 |
| Power & GND | Common with servo supply |

### Build & Upload
1. **Arduino IDE** → Install ESP8266 core (Board Manager URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`).  
2. Install **ESP8266 LittleFS Data Upload** tool (available via Library Manager).  
3. Libraries needed: `ESP8266WiFi`, `ESP8266WebServer`, `Servo` (bundled).  
4. Open `KabelMarker_Pro.ino`.  
5. Tools → Board: select your ESP8266 board (e.g. *LOLIN(WEMOS) D1 mini*).  
6. **Upload Sketch**.  
7. **Upload LittleFS Data** (this copies the `data/` folder to the SPIFFS/LittleFS partition).  

### Usage
1. Power the device.  
2. Connect to Wi‑Fi: **KabelMarker_Pro** (open network).  
3. Visit **http://192.168.4.1** in any browser.  
4. Press **MARK**. The servo moves from **20° → 110°** (adjustable), holds, and returns.  
5. A buzzer melody confirms completion.  

### Tuning
Edit these constants in the sketch:
- `DEFAULT_START_ANGLE` – home position  
- `DEFAULT_END_ANGLE` – marking position  
- `HOLD_TIME` – dwell time at end angle (ms)  
- `DEFAULT_SPEED` – currently fixed; stepping rate is defined by `SERVO_UPDATE_INTERVAL`

### Credits
- Design & code fingerprint embedded: **#Amir Mobasheraghdam**

---

## 🇩🇪 Deutsch

### Funktionen
- Eigenständiger Wi‑Fi Access Point (offen) für sofortigen Zugriff  
- Große, mobile‑optimierte Oberfläche (iPhone‑tauglich)  
- Schnelle Servobewegung mit adaptiver Schrittsteuerung  
- Akustische Bestätigung nach Abschluss  
- Physischer Trigger über D7–GND (Taster oder Relais)  

### Hardware
| Komponente | Pin |
|------------|-----|
| Servo‑Signal | D5 |
| Summer | D6 |
| Status‑LED | eingebaut |
| Externer Taster (NO gegen GND) | D7 |
| Strom & GND | gemeinsam mit Servo‑Versorgung |

### Kompilieren & Hochladen
1. **Arduino IDE** → ESP8266‑Core installieren (Board‑Manager‑URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`).  
2. **ESP8266 LittleFS Data Upload**‑Tool installieren (über Bibliotheksverwalter).  
3. Benötigte Bibliotheken: `ESP8266WiFi`, `ESP8266WebServer`, `Servo` (im Core enthalten).  
4. `KabelMarker_Pro.ino` öffnen.  
5. Werkzeuge → Board: das passende ESP8266‑Board auswählen (z. B. *LOLIN(WEMOS) D1 mini*).  
6. **Sketch hochladen**.  
7. **LittleFS‑Daten hochladen** (dies überträgt den Ordner `data/` in das LittleFS‑Dateisystem).  

### Bedienung
1. Gerät mit Strom versorgen.  
2. Mit dem WLAN **KabelMarker_Pro** (offenes Netzwerk) verbinden.  
3. Im Browser **http://192.168.4.1** aufrufen.  
4. Auf **MARKIEREN** klicken. Der Servo bewegt sich von **20° → 110°** (einstellbar), hält und kehrt zurück.  
5. Eine Summer‑Melodie bestätigt das Ende.  

### Anpassung
Folgende Konstanten im Sketch können geändert werden:
- `DEFAULT_START_ANGLE` – Ausgangsposition  
- `DEFAULT_END_ANGLE` – Markierposition  
- `HOLD_TIME` – Verweilzeit in Endposition (ms)  
- `DEFAULT_SPEED` – aktuell fest; Schrittgeschwindigkeit wird über `SERVO_UPDATE_INTERVAL` gesteuert

### Credits
- Design & Code‑Fingerprint eingebettet: **#Amir Mobasheraghdam**

---

## License & Attribution
This project is open‑source. Please retain the original author credits.  
**#Amir Mobasheraghdam** – *KabelMarker Pro*
