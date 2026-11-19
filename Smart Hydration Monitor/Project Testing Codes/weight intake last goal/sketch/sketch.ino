#define BLYNK_TEMPLATE_ID "TMPL3Jy0mHmaB"
#define BLYNK_TEMPLATE_NAME "Smart Hydration"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// WiFi
char ssid[] = "Nayakwadi";
char pass[] = "8530812055";

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
float lastDrink = 0;
float goal = 2000;

String bottleStatus = "Starting...";

BlynkTimer timer;

// -------- GET STABLE WEIGHT --------
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

  // FIRST VALUE SET
  if (prevWeight == 0) {
    prevWeight = weight;
    Serial.println("System Ready");
  }

  float diff = prevWeight - weight;

  lastDrink = 0;

  // -------- DRINK DETECTION --------
  if (diff > 0.5 && diff < 300) {
    totalIntake += diff;
    lastDrink = diff;

    Serial.print("Drank: ");
    Serial.println(diff);
  }

  // -------- REFILL DETECTION --------
  if (weight > prevWeight + 10) {
    Serial.println("Refill Detected");
  }

  prevWeight = weight;

  // -------- BOTTLE STATUS --------
  if (weight < 200) {
    bottleStatus = "Empty";
  } 
  else if (weight >= 200 && weight < 800) {
    bottleStatus = "Medium";
  } 
  else {
    bottleStatus = "Full";
  }

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
  Serial.print("Last Drink: "); Serial.println(lastDrink);
  Serial.print("Total Intake: "); Serial.println(totalIntake);
  Serial.print("Bottle Status: "); Serial.println(bottleStatus);

  // -------- SEND TO BLYNK --------
  Blynk.virtualWrite(V0, weight);
  Blynk.virtualWrite(V2, totalIntake);
  Blynk.virtualWrite(V3, percent);
  Blynk.virtualWrite(V4, status);

  Blynk.virtualWrite(V5, lastDrink);
  Blynk.virtualWrite(V6, String(bottleStatus));  // IMPORTANT FIX
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