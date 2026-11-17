#define BLYNK_TEMPLATE_ID "TMPL3Jy0mHmaB"
#define BLYNK_TEMPLATE_NAME "Smart Hydration"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// WiFi
char ssid[] = "Admin";
char pass[] = "Password";

// HX711
#define DT D2
#define SCK D3
HX711 scale;

// Calibration
float calibration_factor = -16400;

// Variables
float weight = 0;
float prevWeight = 0;
float totalIntake = 0;
float goal = 2000;

BlynkTimer timer;

// -------- STABLE WEIGHT --------
float getWeight() {
  float sum = 0;

  for (int i = 0; i < 20; i++) {
    sum += scale.get_units();
    delay(3);
  }

  float avg = sum / 20;

  if (avg < 0) avg = 0;

  return avg;
}

// -------- GOAL --------
BLYNK_WRITE(V1) {
  goal = param.asFloat();
  if (goal < 100) goal = 100;
}

// -------- MAIN --------
void sendData() {

  if (!scale.is_ready()) {
    Serial.println("HX711 ERROR");
    return;
  }

  weight = getWeight();

  // FIRST VALUE SET
  if (prevWeight == 0) {
    prevWeight = weight;
    Serial.println("System Ready");
    return;
  }

  float diff = prevWeight - weight;

  // 🔥 PRINT DEBUG (IMPORTANT)
  Serial.println("------");
  Serial.print("Weight: "); Serial.println(weight);
  Serial.print("Prev: "); Serial.println(prevWeight);
  Serial.print("Diff: "); Serial.println(diff);

  // -------- DRINK DETECTION (VERY SENSITIVE) --------
  if (diff > 0.2 && diff < 500) {
    totalIntake += diff;

    Serial.print("Drank: ");
    Serial.println(diff);
  }

  // -------- REFILL DETECTION --------
  if (weight > prevWeight + 10) {
    Serial.println("Refill");
    prevWeight = weight;
    return;
  }

  prevWeight = weight;

  // -------- PERCENT --------
  float percent = (totalIntake / goal) * 100;
  if (percent > 100) percent = 100;

  // -------- STATUS --------
  String status;
  if (percent < 30) status = "Drink Water!";
  else if (percent < 80) status = "Keep Going";
  else status = "Goal Completed";

  // -------- PRINT FINAL --------
  Serial.print("Total Intake: "); Serial.println(totalIntake);
  Serial.print("Percent: "); Serial.println(percent);

  // -------- BLYNK --------
  Blynk.virtualWrite(V0, weight);
  Blynk.virtualWrite(V2, totalIntake);
  Blynk.virtualWrite(V3, percent);
  Blynk.virtualWrite(V4, status);
}

// -------- SETUP --------
void setup() {
  Serial.begin(9600);

  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  delay(3000);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, sendData);
}

// -------- LOOP --------
void loop() {
  Blynk.run();
  timer.run();
}