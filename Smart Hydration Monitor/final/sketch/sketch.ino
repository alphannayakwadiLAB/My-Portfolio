#define BLYNK_TEMPLATE_ID "TMPL3Jy0mHmaB"
#define BLYNK_TEMPLATE_NAME "Smart Hydration"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// -------- WIFI --------
char ssid[] = "Nayakwadi";
char pass[] = "8530812055";

// -------- HX711 --------
#define DT D2
#define SCK D3
HX711 scale;

// -------- CALIBRATION --------
float calibration_factor = -16400;  // change this after calibration

// -------- VARIABLES --------
float weight = 0;
float stableWeight = 0;
float initialWeight = -1;   // FIX: start with -1
float consumed = 0;
float goal = 2000;          // default goal (ml)

BlynkTimer timer;

// -------- STABLE WEIGHT FUNCTION --------
float getWeight() {
  float sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += scale.get_units();
    delay(5);
  }

  float avg = sum / 10;

  if (avg < 0) avg = 0;

  // smoothing filter
  stableWeight = (0.7 * stableWeight) + (0.3 * avg);

  return stableWeight;
}

// -------- RECEIVE GOAL FROM BLYNK --------
BLYNK_WRITE(V1) {
  goal = param.asFloat();

  if (goal < 100) goal = 100;  // safety
}

// -------- MAIN FUNCTION --------
void sendData() {

  if (!scale.is_ready()) {
    Serial.println("HX711 ERROR");
    return;
  }

  weight = getWeight();

  // FIX: Set initial weight correctly
  if (initialWeight < 0 && weight > 20) {
    initialWeight = weight;
    Serial.println("Initial Weight Set");
  }

  // Calculate consumed
  consumed = initialWeight - weight;

  if (consumed < 0) consumed = 0;

  // Percentage
  float percent = (consumed / goal) * 100;

  if (percent > 100) percent = 100;

  // Status
  String status;

  if (percent < 30) status = "Drink Water!";
  else if (percent < 80) status = "Keep Going";
  else status = "Goal Completed";

  // Debug
  Serial.println("------");
  Serial.print("Weight: "); Serial.println(weight);
  Serial.print("Consumed: "); Serial.println(consumed);
  Serial.print("Goal: "); Serial.println(goal);
  Serial.print("Percent: "); Serial.println(percent);

  // Send to Blynk
  Blynk.virtualWrite(V0, weight);
  Blynk.virtualWrite(V2, consumed);
  Blynk.virtualWrite(V3, percent);
  Blynk.virtualWrite(V4, status);
}

// -------- SETUP --------
void setup() {
  Serial.begin(9600);

  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, sendData);
}

// -------- LOOP --------
void loop() {
  Blynk.run();
  timer.run();
}