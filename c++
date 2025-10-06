// KabelMarker Pro — ESP8266 (NodeMCU / Wemos D1 mini)
// Triggers: /move over Wi-Fi OR physical: D7 shorted to GND.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ===== Wi-Fi AP =====
const char* AP_SSID = "KabelMarker_Pro";
const char* AP_PASS = ""; // open

// ===== Pins =====
#define SERVO_PIN       D5
#define BUZZER_PIN      D6
#define STATUS_LED      LED_BUILTIN
#define START_SENSE_PIN D7   // button/relay NO -> D7, COM -> GND

// ===== Servo params =====
#define DEFAULT_START_ANGLE  20
#define DEFAULT_END_ANGLE    110
#define DEFAULT_SPEED        10
#define HOLD_TIME            140
#define MIN_ANGLE            0
#define MAX_ANGLE            160

// ===== Timing =====
#define DEBOUNCE_TIME_MS       200
#define DEBOUNCE_WIFI_MS       400
#define SERVO_UPDATE_INTERVAL  6
#define BUZZER_PAUSE           80

// ===== Buzzer seq =====
const uint16_t BUZZER_SEQ[] = {1000,100, 1200,100, 1500,200};

// ===== State =====
struct {
  uint8_t startAngle = DEFAULT_START_ANGLE;
  uint8_t endAngle   = DEFAULT_END_ANGLE;
  uint8_t speed      = DEFAULT_SPEED;
  bool isMoving = false;
  uint32_t lastPhysicalTrigger = 0;
  uint32_t lastWifiTrigger = 0;
  uint32_t lastServoUpdate = 0;
} state;

Servo markerServo;
ESP8266WebServer server(80);

// ISR flag only
volatile bool physicalEdgeFlag = false;
ICACHE_RAM_ATTR void onPhysicalFalling(){ physicalEdgeFlag = true; }

// ===== HTML =====
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>KabelMarker Pro</title>
<style>
body{font-family:sans-serif;text-align:center;padding:20px;}
button{font-size:20px;padding:18px 28px;border-radius:12px;background:#ff6f00;color:#fff;border:none;}
</style>
</head><body>
<h1>KabelMarker Pro</h1>
<p>Status: <span id="st">Ready</span></p>
<button onclick="go()">MARK</button>
<script>
async function go(){
  document.getElementById('st').textContent='Working...';
  try{ await fetch('/move'); }catch(e){ alert('Error'); }
  setTimeout(()=>document.getElementById('st').textContent='Ready',600);
}
</script>
</body></html>
)rawliteral";

// ===== Helpers =====
static inline uint8_t clampAngle(int v){
  if(v < MIN_ANGLE) return MIN_ANGLE;
  if(v > MAX_ANGLE) return MAX_ANGLE;
  return (uint8_t)v;
}
void beepSequence(){
  for(size_t i=0;i<sizeof(BUZZER_SEQ)/sizeof(uint16_t); i+=2){
    tone(BUZZER_PIN, BUZZER_SEQ[i], BUZZER_SEQ[i+1]);
    delay(BUZZER_SEQ[i+1] + BUZZER_PAUSE);
  }
}
void moveServoCycle(){
  if(state.isMoving) return;
  state.isMoving = true;
  digitalWrite(STATUS_LED, LOW);

  int current = markerServo.read();
  int target  = clampAngle(state.endAngle);
  while(current != target){
    if(millis() - state.lastServoUpdate >= SERVO_UPDATE_INTERVAL){
      int dir = (target > current) ? 1 : -1;
      current += dir; markerServo.write(current);
      state.lastServoUpdate = millis();
    }
    server.handleClient(); yield();
  }

  delay(HOLD_TIME);

  target  = clampAngle(state.startAngle);
  current = markerServo.read();
  while(current != target){
    if(millis() - state.lastServoUpdate >= SERVO_UPDATE_INTERVAL){
      int dir = (target > current) ? 1 : -1;
      current += dir; markerServo.write(current);
      state.lastServoUpdate = millis();
    }
    server.handleClient(); yield();
  }

  beepSequence();
  digitalWrite(STATUS_LED, HIGH);
  state.isMoving = false;
}

// ===== Web =====
void handleRoot(){ server.send_P(200, "text/html", HTML_PAGE); }
void handleMove(){
  if(millis() - state.lastWifiTrigger < DEBOUNCE_WIFI_MS){
    server.send(429, "text/plain", "Too Many Requests"); return;
  }
  state.lastWifiTrigger = millis();
  if(!state.isMoving){ server.send(200, "text/plain", "OK");  moveServoCycle(); }
  else               { server.send(200, "text/plain", "BUSY"); }
}

// ===== Setup =====
void setup(){
  Serial.begin(115200); Serial.println(); Serial.println("=== KabelMarker Pro boot ===");
  pinMode(STATUS_LED, OUTPUT); digitalWrite(STATUS_LED, HIGH);
  pinMode(BUZZER_PIN, OUTPUT);

  // Physical trigger: D7 with internal pull-up, falling to GND
  pinMode(START_SENSE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(START_SENSE_PIN), onPhysicalFalling, FALLING);
  Serial.println("Physical trigger on D7 (to GND) armed.");

  // Servo
  markerServo.attach(SERVO_PIN, 544, 2400);
  markerServo.write(state.startAngle);

  // Wi-Fi AP
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.printf("AP SSID: %s\n", AP_SSID);
  Serial.print("AP IP   : "); Serial.println(WiFi.softAPIP());

  // Web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/move", HTTP_GET, handleMove);
  server.begin();
  Serial.println("HTTP server started.");

  // Startup beep
  tone(BUZZER_PIN, 1500, 80); delay(120); tone(BUZZER_PIN, 2000, 60);
}

// ===== Loop =====
void loop(){
  server.handleClient();

  if(physicalEdgeFlag){
    physicalEdgeFlag = false;
    uint32_t now = millis();
    bool lowNow = (digitalRead(START_SENSE_PIN) == LOW);
    if(lowNow && (now - state.lastPhysicalTrigger > DEBOUNCE_TIME_MS)){
      state.lastPhysicalTrigger = now;
      Serial.println("Physical trigger accepted (D7->GND).");
      if(!state.isMoving) moveServoCycle();
      else Serial.println("Ignored: servo busy.");
    } else {
      Serial.printf("Edge ignored (low=%d, dt=%lu ms)\n", lowNow?1:0, (unsigned long)(now - state.lastPhysicalTrigger));
    }
  }

  // Debug every 500 ms
  static uint32_t dbgT=0;
  if(millis()-dbgT>500){ dbgT=millis(); Serial.printf("D7=%d  moving=%d\n", digitalRead(START_SENSE_PIN), state.isMoving?1:0); }

  // Blink when idle
  static uint32_t lastBlink=0;
  if(!state.isMoving && millis()-lastBlink>800){
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    lastBlink = millis();
  }

  yield();
}
