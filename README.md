# servo Pro
Fast cable marker controller on **ESP8266** with on-device web UI.  
_No external app required. Just connect to the AP and press **MARK**._

> Ownership tag: **#Amir Mobasheraghdam** (embedded in source & assets)

## Features
- Standalone Wi‑Fi Access Point (open) for instant access
- Large, mobile-first UI (iPhone-ready)
- Fast servo motion with adaptive stepping
- Audible confirmation tones on completion

## Hardware
- Board: ESP8266 (e.g., Wemos D1 mini)
- Servo signal: **D5**
- Buzzer: **D6**
- Common GND shared with servo power
- LED: built-in status LED

## Build & Upload
1. **Arduino IDE** → Install ESP8266 core.
2. Install **ESP8266 LittleFS Data Upload** tool.
3. Libraries: `ESP8266WiFi`, `ESP8266WebServer`, `LittleFS`, `Servo` (bundled).
4. Open `src/KabelMarker_Pro.ino`.
5. Tools → Board: “ESP8266 …”.
6. **Upload Sketch**.
7. **Upload LittleFS Data** (uploads `data/` folder).

## Use
1. Power the device.
2. Join Wi‑Fi: **KabelMarker_Pro** (open).
3. Navigate to **http://192.168.4.1**.
4. Press **MARK**. The servo goes from **20° → 162°**, holds, and returns.

## Tuning
- Angles: `START_ANGLE`, `END_ANGLE` in the sketch.
- Speed behavior is already optimized (no UI changes needed).

## Credits
- Design & code fingerprint embedded: **#Amir Mobasheraghdam**
