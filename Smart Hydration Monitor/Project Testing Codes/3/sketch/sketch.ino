#define BLYNK_TEMPLATE_ID "TMPLxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Smart Hydration Monitor"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// ---------------- WIFI ----------------
char ssid[] = "Nayakwadi";
char pass[] = "8530812055";

// ---------------- HX711 ----------------
#define DT D5
#define SCK D6

HX711 scale;

// 🔥 IMPORTANT: set your calibration factor
float calibration_factor = -7350;

// ---------------- FILTER BUFFER ----------------
float readings[10];
int readIndex = 0;

// ---------------- VARIABLES ----------------
float last_weight = 0;
float total_water = 0;
float last_drink = 0;

BlynkTimer timer;

// ---------------- STABLE WEIGHT FUNCTION ----------------
float getStableWeight() {

  readings[readIndex] = scale.get_units(1);
  readIndex = (readIndex + 1) % 10;

  float sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += readings[i];
  }

  return sum / 10.0;
}

// ---------------- SEND DATA ----------------
void sendData() {

  if (!scale.is_ready()) {
    Serial.println("HX711 not ready");
    return;
  }

  float weight = getStableWeight();   // grams

  // 🔥 noise filter (ignore small changes)
  float diff = last_weight - weight;

  if (diff > 20) {   // 20g threshold
    last_drink = diff / 1000.0;
    total_water += last_drink;
  } else {
    last_drink = 0;
  }

  last_weight = weight;

  float litres = weight / 1000.0;

  int percent = (litres / 2.0) * 100;
  if (percent > 100) percent = 100;
  if (percent < 0) percent = 0;

  String status;
  if (weight > 1700) status = "FULL";
  else if (weight > 700) status = "MEDIUM";
  else status = "LOW";

  // ---------------- SEND TO BLYNK ----------------
  Blynk.virtualWrite(V0, weight);       // weight in grams
  Blynk.virtualWrite(V1, litres);       // litres
  Blynk.virtualWrite(V2, last_drink);   // last intake
  Blynk.virtualWrite(V3, percent);      // percentage
  Blynk.virtualWrite(V4, status);       // status text

  // ---------------- SERIAL MONITOR ----------------
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.print(" g | Litres: ");
  Serial.print(litres);
  Serial.print(" | Status: ");
  Serial.println(status);
}

// ---------------- SETUP ----------------
void setup() {

  Serial.begin(9600);

  scale.begin(DT, SCK);
  delay(1000);

  scale.set_scale(calibration_factor);
  scale.tare();   // zero scale

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(3000L, sendData); // slow stable updates
}

// ---------------- LOOP ----------------
void loop() {
  Blynk.run();
  timer.run();
}