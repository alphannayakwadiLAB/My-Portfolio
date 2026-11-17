#include "HX711.h"

// HX711 pins
#define DT 3
#define SCK 2

HX711 scale;

float calibration_factor = -1000; // Change this after calibration

void setup() {
  Serial.begin(9600);
  
  scale.begin(DT, SCK);

  Serial.println("Initializing...");
  delay(2000);

  // Reset scale
  scale.set_scale();
  scale.tare();  // Reset weight to 0

  Serial.println("Tare complete");
}

void loop() {
  // Read weight
  float weight = scale.get_units(10); // average of 10 readings

  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" grams");

  delay(1000);
}