/* ESP32 + Soil Moisture + Relay + Blynk
   Behavior:
   - Show soil moisture on V0.
   - Manual motor control via button V1 (Switch).
   - If motor is ON and moisture >= moistureOffThreshold -> auto OFF.
   - Motor will not restart while moisture >= moistureOffThreshold.
   - Motor can be restarted once moisture < moistureResetThreshold (hysteresis).
*/

#define BLYNK_TEMPLATE_ID "TMPL3NFPutRMz"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "SLSrhA9p9moeIP773Ta76qLZBwpFIGJ_"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "iPhone";
char pass[] = "atharva0909";

// Hardware pins (adjust if needed)
#define MOISTURE_PIN 34   // ADC pin (input-only pins ok: 34..39)
#define RELAY_PIN    25   // Relay control pin (use safe GPIO like 23/25/26/27)

// Program parameters
const bool relayActiveLow = true;     // true = LOW drives relay ON (common modules). Set false if opposite.
const int moistureOffThreshold = 80;  // % threshold to auto-turn OFF
const int moistureResetThreshold = 75; // % threshold to allow restart (hysteresis)
const unsigned long readIntervalMs = 2000; // sensor read interval

// State variables
int moistureRaw = 0;
int moisturePercent = 0;
int motorState = 0;       // 0 = OFF, 1 = ON
bool autoStopped = false; // true if auto-stopped due to high moisture

BlynkTimer timer;

// Utility: set relay to ON/OFF considering relay logic
void setRelay(bool on) {
  if (relayActiveLow) digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  else               digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}

// Read moisture, update Blynk, and enforce auto-off / hysteresis
void readSoilMoisture() {
  moistureRaw = analogRead(MOISTURE_PIN); // 0..4095 (ESP32 default ADC)
  // Map raw to percentage. Try this mapping first; if your sensor is inverted, swap 0/4095.
  moisturePercent = map(moistureRaw, 0, 4095, 0, 100);

  // If your sensor produces 0 = wet and 4095 = dry, use:
  // moisturePercent = map(moistureRaw, 4095, 0, 0, 100);

  Serial.print("Raw: "); Serial.print(moistureRaw);
  Serial.print("  Moisture: "); Serial.print(moisturePercent); Serial.println("%");

  Blynk.virtualWrite(V0, moisturePercent);

  // Auto-OFF logic
  if (motorState == 0 && moisturePercent >= moistureOffThreshold) {
    // Turn OFF motor
    setRelay(false);        // OFF
    motorState = 1;
    autoStopped = true;
    Blynk.virtualWrite(V1, 0); // update button in app
    Blynk.logEvent("motor_auto_off", "Soil wet — Motor turned OFF automatically");
    Serial.println("AUTO OFF: Moisture >= threshold, motor stopped");
  }

  // Clear autoStopped when moisture falls below reset threshold
  if (autoStopped && moisturePercent < moistureResetThreshold) {
    autoStopped = false;
    Serial.println("Moisture below reset threshold — autoStopped cleared (motor may be restarted)");
    Blynk.logEvent("motor_auto_cleared", "Soil dried — Motor restart allowed");
  }
}

// Blynk button V1 (Switch). Called whenever widget changes state.
BLYNK_WRITE(V1) {
  int v = param.asInt();
  Serial.print("Blynk V1: "); Serial.println(v);

  if (v == 1) { // user requested motor ON
    if (autoStopped) {
      // Reject start because moisture too high
      Serial.println("Start blocked: motor auto-stopped due to high moisture");
      // Force the app button back to OFF
      Blynk.virtualWrite(V1, 0);
      return;
    }
    // Start motor
    setRelay(true);
    motorState = 0;
    Serial.println("Motor turned ON (manual)");
  } else { // v == 0 => stop request
    setRelay(false);
    motorState = 1;
    Serial.println("Motor turned OFF (manual)");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(RELAY_PIN, OUTPUT);
  // ensure motor off initially
  setRelay(false);

  pinMode(MOISTURE_PIN, INPUT);

  Serial.println();
  Serial.println("Starting Blynk + Moisture control");
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(readIntervalMs, readSoilMoisture);
}

void loop() {
  Blynk.run();
  timer.run();
}
