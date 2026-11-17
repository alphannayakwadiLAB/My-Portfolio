#define BLYNK_TEMPLATE_ID "TMPL3Jy0mHmaB"
#define BLYNK_TEMPLATE_NAME "Smart Hydration Meter"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// WiFi
char ssid[] = "Nayakwadi";
char pass[] = "8530812055";

// HX711 pins
#define DT D2
#define SCK D3

HX711 scale;

// Calibration factor
float calibration_factor = -7050;

// Variables
float weight = 0;
float lastWeight = 0;
float intake = 0;
float goal = 2000;
float stableWeight = 0;

BlynkTimer timer;

// ---------------- BLYNK ----------------
BLYNK_WRITE(V1) {
  goal = param.asFloat();
  Serial.println("Goal Updated");
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    intake = 0;
    Serial.println("Intake Reset");
  }
}

// ---------------- READ FUNCTION ----------------
void readWeight() {

  if (!scale.is_ready()) {
    Serial.println("HX711 NOT FOUND ❌");
    return;
  }

  float raw = scale.get_units(10);

  if (raw < 0) raw = 0;

  stableWeight = (stableWeight * 0.8) + (raw * 0.2);
  weight = stableWeight;

  float diff = lastWeight - weight;

  if (diff > 5 && diff < 500) {
    intake += diff;
    Serial.print("Drank: ");
    Serial.println(diff);
  }

  lastWeight = weight;

  Blynk.virtualWrite(V0, weight);
  Blynk.virtualWrite(V3, intake);
  Blynk.virtualWrite(V4, goal);

  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.print(" | Intake: ");
  Serial.println(intake);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("System Starting...");

  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  Serial.println("HX711 Ready");

  // WiFi connect
  WiFi.begin(ssid, pass);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected ✅");

  // Blynk connect (non-blocking)
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  Serial.println("Blynk Connected ✅");

  timer.setInterval(1000L, readWeight);
}

// ---------------- LOOP ----------------
void loop() {
  Blynk.run();
  timer.run();
}