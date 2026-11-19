#define BLYNK_TEMPLATE_ID "TMPLxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Smart Hydration Monitor"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// WiFi credentials
char ssid[] = "Nayakwadi";
char pass[] = "8530812055";

// HX711 pins
#define DT D5
#define SCK D6

HX711 scale;

// 🔧 PUT YOUR CALIBRATED VALUE HERE
float calibration_factor = -7350;   // <-- change this

// Hydration variables
float current_weight = 0;
float previous_weight = 0;
float water_intake = 0;
float last_drink = 0;

// Daily goal (grams = ml)
float daily_goal = 2000; // 2 liters

BlynkTimer timer;

// 📌 Main function
void readHydration() {

  if (scale.is_ready()) {

    current_weight = scale.get_units(10);

    Serial.print("Weight: ");
    Serial.println(current_weight);

    // Detect drinking
    if (current_weight < previous_weight - 5) {

      last_drink = previous_weight - current_weight;
      water_intake += last_drink;

      Serial.print("Last Drink: ");
      Serial.println(last_drink);
    }

    previous_weight = current_weight;

    // Calculate goal %
    int goal_percent = (water_intake / daily_goal) * 100;
    if (goal_percent > 100) goal_percent = 100;

    // Bottle status
    String status;
    if (current_weight > 1000) status = "Full";
    else if (current_weight > 300) status = "Medium";
    else status = "Low";

    // Send to Blynk
    Blynk.virtualWrite(V0, current_weight);   // Weight
    Blynk.virtualWrite(V1, water_intake);     // Total Intake
    Blynk.virtualWrite(V2, last_drink);       // Last Drink
    Blynk.virtualWrite(V3, goal_percent);     // Goal %
    Blynk.virtualWrite(V4, status);           // Bottle Status

  } else {
    Serial.println("HX711 not ready");
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Smart Hydration System Starting...");

  // HX711 setup
  scale.begin(DT, SCK);
  delay(500);

  if (!scale.is_ready()) {
    Serial.println("HX711 NOT FOUND!");
    while (1);
  }

  scale.set_scale(calibration_factor);
  scale.tare();

  Serial.println("HX711 Ready & Calibrated");

  // Connect Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Run every 1 second
  timer.setInterval(1000L, readHydration);
}

void loop() {
  Blynk.run();
  timer.run();
}