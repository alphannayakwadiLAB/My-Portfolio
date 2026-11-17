#include <Servo.h>

// Define pins
const int sensor0 = 2;     // Hall sensor at 0°
const int sensor90 = 3;    // Hall sensor at 90°
const int sensor180 = 4;   // Hall sensor at 180°
const int servoPin = 9;    // Servo signal pin

Servo regulatorServo;

void setup() {
  pinMode(sensor0, INPUT);
  pinMode(sensor90, INPUT);
  pinMode(sensor180, INPUT);

  regulatorServo.attach(servoPin);
  regulatorServo.write(0);  // Start in OFF position

  Serial.begin(9600);
}

void loop() {
  // Read sensor states
  bool is0On = digitalRead(sensor0) == LOW;      // Active LOW
  bool is90On = digitalRead(sensor90) == LOW;
  bool is180On = digitalRead(sensor180) == LOW;

  if (is0On) {
    // Knob at 0° → Turn gas OFF
    regulatorServo.write(0);
    Serial.println("Knob at 0° → Regulator OFF");
  } else if (is90On || is180On) {
    // Knob at 90° or 180° → Turn gas ON
    regulatorServo.write(180);
    Serial.println("Knob at 90° or 180° → Regulator ON");
  } else {
    // No sensor active — optional: keep previous state
    Serial.println("No magnet detected — no change");
  }

  delay(200); // Small delay for stability
}
