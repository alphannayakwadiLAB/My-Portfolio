#define BLYNK_TEMPLATE_ID "TMPL3NFPutRMz"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "SLSrhA9p9moeIP773Ta76qLZBwpFIGJ_"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "SLSrhA9p9moeIP773Ta76qLZBwpFIGJ_";
char ssid[] = "JioPrivateNet";
char pass[] = "1020304050";

#define MOISTURE_PIN 34
#define RELAY_PIN 25
#define SWITCH_ON 33
#define SWITCH_OFF 32
#define LED_ON_PIN 26
#define LED_OFF_PIN 27

int moistureValue = 0;
int moisturePercent = 0;
int motorState = 0;

BlynkTimer timer;

// 🔹 Read soil moisture
void readSoilMoisture() {
  moistureValue = analogRead(MOISTURE_PIN);
  moisturePercent = map(moistureValue, 4095, 1000, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Blynk.virtualWrite(V0, moisturePercent);
  Serial.print("Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  // 🔹 Auto OFF motor if soil wet
  if (motorState == 1 && moisturePercent >= 80) {
    digitalWrite(RELAY_PIN, LOW); // Active LOW relay → OFF
    motorState = 0;
    updateMotorStatus();
    Blynk.virtualWrite(V1, 0);
    Serial.println(" Motor auto-stopped: Soil wet");
  }
}

// 🔹 Update LED indicators
void updateMotorStatus() {
  if (motorState == 1) {
    digitalWrite(LED_ON_PIN, HIGH);
    digitalWrite(LED_OFF_PIN, LOW);
  } else {
    digitalWrite(LED_ON_PIN, LOW);
    digitalWrite(LED_OFF_PIN, HIGH);
  }
}

// 🔹 Blynk button control
BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    digitalWrite(RELAY_PIN, HIGH);  // Active LOW relay → ON
    motorState = 1;
    Serial.println("Motor ON (Blynk)");
  } else {
    digitalWrite(RELAY_PIN, LOW); // OFF
    motorState = 0;
    Serial.println("Motor OFF (Blynk)");
  }
  updateMotorStatus();
}

// 🔹 Check hardware switches
void readSwitches() {
  if (digitalRead(SWITCH_ON) == HIGH) {
    digitalWrite(RELAY_PIN, HIGH);
    motorState = 1;
    Blynk.virtualWrite(V1, 1);
    updateMotorStatus();
    Serial.println("Motor ON (Switch)");
  }
  if (digitalRead(SWITCH_OFF) == HIGH) {
    digitalWrite(RELAY_PIN, LOW);
    motorState = 0;
    Blynk.virtualWrite(V1, 0);
    updateMotorStatus();
    Serial.println("Motor OFF (Switch)");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Motor OFF initially

  pinMode(MOISTURE_PIN, INPUT);
  pinMode(SWITCH_ON, INPUT);
  pinMode(SWITCH_OFF, INPUT);
  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_OFF_PIN, OUTPUT);

  digitalWrite(LED_ON_PIN, LOW);
  digitalWrite(LED_OFF_PIN, HIGH);

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, readSoilMoisture);
  timer.setInterval(100L, readSwitches);
}

void loop() {
  Blynk.run();
  timer.run();
}
