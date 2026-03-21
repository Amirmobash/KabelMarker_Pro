// KabelMarker Pro — ESP8266 (NodeMCU / Wemos D1 mini)
// Autor:         Amir Mobasheraghdam
// Datum:         2025-03-21
// Version:       2.0 (optimiert und lokalisiert)
// Beschreibung:  Bewegt einen Servo (Marker) per Web-Button oder externem Taster (D7 gegen GND).
//                Start- und Endwinkel sowie Geschwindigkeit sind über Konstanten anpassbar.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ===== Wi-Fi Access Point =====
const char* AP_SSID = "KabelMarker_Pro";
const char* AP_PASS = "";   // offenes Netzwerk

// ===== Pinbelegung =====
#define SERVO_PIN       D5    // Servo-Signal
#define BUZZER_PIN      D6    // Piezo-Summer
#define STATUS_LED      LED_BUILTIN  // eingebaute LED (low = an)
#define START_SENSE_PIN D7    // externer Taster (Schließer) gegen GND

// ===== Servo-Parameter =====
#define DEFAULT_START_ANGLE  20   // Startposition (zurück)
#define DEFAULT_END_ANGLE    110  // Endposition (markieren)
#define DEFAULT_SPEED        10   // nicht verwendet (aktuell fest)
#define HOLD_TIME            140  // Verweildauer in Endposition (ms)
#define MIN_ANGLE            0    // minimaler Winkel
#define MAX_ANGLE            160  // maximaler Winkel (Servo-Grenzen)

// ===== Zeitsteuerung =====
#define DEBOUNCE_TIME_MS     200  // Entprellzeit für Taster
#define DEBOUNCE_WIFI_MS     400  // Mindestabstand zwischen Web-Aufrufen
#define SERVO_UPDATE_INTERVAL 6   // Intervall für Servo-Bewegung (ms)
#define BUZZER_PAUSE         80   // Pause zwischen Summer-Tönen (ms)

// ===== Summer-Tonfolge =====
const uint16_t BUZZER_SEQ[] = {1000, 100, 1200, 100, 1500, 200};

// ===== Zustandsvariablen =====
struct {
  uint8_t startAngle = DEFAULT_START_ANGLE;
  uint8_t endAngle   = DEFAULT_END_ANGLE;
  uint8_t speed      = DEFAULT_SPEED;
  bool isMoving      = false;
  uint32_t lastPhysicalTrigger = 0;
  uint32_t lastWifiTrigger     = 0;
  uint32_t lastServoUpdate     = 0;
} state;

Servo markerServo;
ESP8266WebServer server(80);

// ISR-Flag für Hardware-Trigger
volatile bool physicalEdgeFlag = false;
ICACHE_RAM_ATTR void onPhysicalFalling() { physicalEdgeFlag = true; }

// ===== HTML-Seite (deutsch) =====
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>KabelMarker Pro</title>
<style>
body{font-family:sans-serif;text-align:center;padding:20px;}
button{font-size:20px;padding:18px 28px;border-radius:12px;background:#ff6f00;color:#fff;border:none;}
</style>
</head><body>
<h1>KabelMarker Pro</h1>
<p>Status: <span id="st">Bereit</span></p>
<button onclick="go()">MARKIEREN</button>
<script>
async function go(){
  document.getElementById('st').textContent='Arbeite...';
  try{ await fetch('/move'); } catch(e){ alert('Fehler'); }
  setTimeout(()=>document.getElementById('st').textContent='Bereit',600);
}
</script>
</body></html>
)rawliteral";

// ===== Hilfsfunktionen =====
static inline uint8_t clampAngle(int v) {
  if (v < MIN_ANGLE) return MIN_ANGLE;
  if (v > MAX_ANGLE) return MAX_ANGLE;
  return (uint8_t)v;
}

void beepSequence() {
  for (size_t i = 0; i < sizeof(BUZZER_SEQ) / sizeof(uint16_t); i += 2) {
    tone(BUZZER_PIN, BUZZER_SEQ[i], BUZZER_SEQ[i + 1]);
    delay(BUZZER_SEQ[i + 1] + BUZZER_PAUSE);
  }
}

// Servozyklus: fährt zur Endposition, hält, fährt zurück und quittiert mit Summer
void moveServoCycle() {
  if (state.isMoving) return;
  state.isMoving = true;
  digitalWrite(STATUS_LED, LOW);   // LED ein (aktiv)

  int current = markerServo.read();
  int target  = clampAngle(state.endAngle);

  // Hinbewegung
  while (current != target) {
    if (millis() - state.lastServoUpdate >= SERVO_UPDATE_INTERVAL) {
      int dir = (target > current) ? 1 : -1;
      current += dir;
      markerServo.write(current);
      state.lastServoUpdate = millis();
    }
    server.handleClient();
    yield();   // Hintergrundaufgaben erlauben
  }

  delay(HOLD_TIME);

  // Rückbewegung
  target  = clampAngle(state.startAngle);
  current = markerServo.read();
  while (current != target) {
    if (millis() - state.lastServoUpdate >= SERVO_UPDATE_INTERVAL) {
      int dir = (target > current) ? 1 : -1;
      current += dir;
      markerServo.write(current);
      state.lastServoUpdate = millis();
    }
    server.handleClient();
    yield();
  }

  beepSequence();
  digitalWrite(STATUS_LED, HIGH);  // LED aus
  state.isMoving = false;
}

// ===== Web-Handler =====
void handleRoot() {
  server.send_P(200, "text/html", HTML_PAGE);
}

void handleMove() {
  if (millis() - state.lastWifiTrigger < DEBOUNCE_WIFI_MS) {
    server.send(429, "text/plain", "Zu viele Anfragen");
    return;
  }
  state.lastWifiTrigger = millis();
  if (!state.isMoving) {
    server.send(200, "text/plain", "OK");
    moveServoCycle();
  } else {
    server.send(200, "text/plain", "BESCHÄFTIGT");
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== KabelMarker Pro gestartet ===");

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  pinMode(BUZZER_PIN, OUTPUT);

  // Hardware-Trigger: D7 mit internem Pull-up, fallende Flanke
  pinMode(START_SENSE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(START_SENSE_PIN), onPhysicalFalling, FALLING);
  Serial.println("Hardware-Trigger an D7 (gegen GND) aktiviert.");

  // Servo initialisieren
  markerServo.attach(SERVO_PIN, 544, 2400);
  markerServo.write(state.startAngle);

  // Wi-Fi Access Point starten
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.printf("Access Point SSID: %s\n", AP_SSID);
  Serial.print("AP IP-Adresse : ");
  Serial.println(WiFi.softAPIP());

  // Webserver einrichten
  server.on("/", HTTP_GET, handleRoot);
  server.on("/move", HTTP_GET, handleMove);
  server.begin();
  Serial.println("HTTP-Server gestartet.");

  // Startsignal: kurzer Summer-Ton
  tone(BUZZER_PIN, 1500, 80);
  delay(120);
  tone(BUZZER_PIN, 2000, 60);
}

// ===== Hauptschleife =====
void loop() {
  server.handleClient();

  // Hardware-Trigger auswerten
  if (physicalEdgeFlag) {
    physicalEdgeFlag = false;
    uint32_t now = millis();
    bool lowNow = (digitalRead(START_SENSE_PIN) == LOW);
    if (lowNow && (now - state.lastPhysicalTrigger > DEBOUNCE_TIME_MS)) {
      state.lastPhysicalTrigger = now;
      Serial.println("Hardware-Trigger akzeptiert (D7->GND).");
      if (!state.isMoving) {
        moveServoCycle();
      } else {
        Serial.println("Ignoriert: Servo beschäftigt.");
      }
    } else {
      Serial.printf("Flanke ignoriert (low=%d, dt=%lu ms)\n", lowNow ? 1 : 0, (unsigned long)(now - state.lastPhysicalTrigger));
    }
  }

  // Debug-Ausgabe alle 500 ms
  static uint32_t dbgT = 0;
  if (millis() - dbgT > 500) {
    dbgT = millis();
    Serial.printf("D7=%d  moving=%d\n", digitalRead(START_SENSE_PIN), state.isMoving ? 1 : 0);
  }

  // Blink-LED im Leerlauf (Statusanzeige)
  static uint32_t lastBlink = 0;
  if (!state.isMoving && millis() - lastBlink > 800) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    lastBlink = millis();
  }

  yield();   // Hintergrundaufgaben
}
