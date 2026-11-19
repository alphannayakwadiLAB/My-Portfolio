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
float calibration_factor = -16400;

// -------- VARIABLES --------
float weight = 0;
float stableWeight = 0;
float lastWeight = 0;
float totalIntake = 0;
float goal = 2000;

// -------- TIMER --------
BlynkTimer timer;

// -------- STABLE WEIGHT FUNCTION --------
float getWeight() {
  float sum = 0;

  for (int i = 0; i < 15; i++) {
    sum += scale.get_units();
    delay(3);
  }

  float avg = sum / 15;

  if (avg < 0) avg = 0;

  // strong smoothing for stability
  stableWeight = (0.9 * stableWeight) + (0.1 * avg);

  return stableWeight;
}

// -------- RECEIVE GOAL --------
BLYNK_WRITE(V1) {
  goal = param.asFloat();
  if (goal < 100) goal = 100;
}

// -------- MAIN FUNCTION --------
void sendData() {

  if (!scale.is_ready()) {
    Serial.println("HX711 ERROR");
    return;
  }

  weight = getWeight();

  // -------- INITIAL SET --------
  if (lastWeight == 0 && weight > 50) {
    lastWeight = weight;
    Serial.println("System Ready");
    return;
  }

  float diff = lastWeight - weight;

  // -------- DRINK DETECTION --------
  if (diff > 0.5 && diff < 300) {
    totalIntake += diff;

    Serial.print("Drank: ");
    Serial.println(diff);
  }

  // -------- REFILL DETECTION --------
  if (weight > lastWeight + 30) {
    Serial.println("Refill Detected");
    lastWeight = weight;
    return;
  }

  lastWeight = weight;

  // -------- SAFETY --------
  if (totalIntake < 0) totalIntake = 0;
  if (totalIntake > 3000) totalIntake = 3000;

  // -------- PERCENT --------
  float percent = (totalIntake / goal) * 100;
  if (percent > 100) percent = 100;

  // -------- STATUS --------
  String status;

  if (percent < 30) status = "Drink Water!";
  else if (percent < 80) status = "Keep Going";
  else status = "Goal Completed";

  // -------- DEBUG --------
  Serial.println("------");
  Serial.print("Weight: "); Serial.println(weight);
  Serial.print("Diff: "); Serial.println(diff);
  Serial.print("Total Intake: "); Serial.println(totalIntake);
  Serial.print("Percent: "); Serial.println(percent);

  // -------- SEND TO BLYNK --------
  Blynk.virtualWrite(V0, weight);        // Weight
  Blynk.virtualWrite(V2, totalIntake);   // Total Intake
  Blynk.virtualWrite(V3, percent);       // Percentage
  Blynk.virtualWrite(V4, status);        // Status
}

// -------- SETUP --------
void setup() {
  Serial.begin(9600);

  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  delay(2000);
  stableWeight = scale.get_units();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, sendData);
}

// -------- LOOP --------
void loop() {
  Blynk.run();
  timer.run();
}