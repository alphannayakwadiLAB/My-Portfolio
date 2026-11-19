#include "HX711.h"

#define DT D5
#define SCK D6

HX711 scale;

float calibration_factor = -7050;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");

  scale.begin(DT, SCK);

  delay(1000); // give HX711 time to stabilize

  if (scale.is_ready()) {
    Serial.println("✅ HX711 Connected");
  } else {
    Serial.println("❌ HX711 NOT FOUND");
    Serial.println("Check wiring!");
    while (1);
  }

  scale.set_scale();
  scale.tare();

  Serial.println("Tare done");
  Serial.println("Place known weight...");
}

void loop() {
  scale.set_scale(calibration_factor);

  float weight = scale.get_units(10);

  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.print(" g | Factor: ");
  Serial.println(calibration_factor);

  // Adjust calibration
  if (Serial.available()) {
    char input = Serial.read();

    if (input == '+') calibration_factor += 10;
    if (input == '-') calibration_factor -= 10;
  }

  delay(1000);
}