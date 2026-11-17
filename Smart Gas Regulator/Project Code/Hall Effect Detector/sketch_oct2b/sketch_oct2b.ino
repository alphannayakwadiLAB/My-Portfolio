#include <Servo.h>

Servo myServo;

// Hall sensor pins
const int hall0 = 2;     // Hall sensor at 0°
const int hall90 = 3;    // Hall sensor at 90°
const int hall180 = 4;   // Hall sensor at 180°

int lastPosition = 0;    // 0 = servo at 0°, 1 = servo at 180°
bool actionDone = false; // prevents repeated motion

void setup() {
  myServo.attach(9);     // Servo signal pin
  pinMode(hall0, INPUT);
  pinMode(hall90, INPUT);
  pinMode(hall180, INPUT);

  myServo.write(0);      // Start at 0°
  Serial.begin(9600);
  Serial.println("System started. Servo at 0°");
}

void loop() {
  int sensor0 = digitalRead(hall0);
  int sensor90 = digitalRead(hall90);
  int sensor180 = digitalRead(hall180);

  // Print sensor states
  Serial.print("Sensor0: "); Serial.print(sensor0);
  Serial.print(" | Sensor90: "); Serial.print(sensor90);
  Serial.print(" | Sensor180: "); Serial.println(sensor180);

  // Condition 1: knob at 90° or 180° → rotate servo +180° (only once)
  if (lastPosition == 0 && (sensor90 == LOW || sensor180 == LOW) && !actionDone) {
    Serial.println("Detected knob at 90°/180°. Rotating servo +180° once...");
    for (int pos = 0; pos <= 180; pos++) {   // gradual sweep
      myServo.write(pos);
      delay(10);
    }
    lastPosition = 1;    // Servo now at 180°
    actionDone = true;
    Serial.println("Servo now at 180°");
  }

  // Condition 2: knob back to 0° → rotate servo -180° (only once)
  if (lastPosition == 1 && sensor0 == LOW && !actionDone) {
    Serial.println("Detected knob at 0°. Rotating servo -180° once...");
    for (int pos = 180; pos >= 0; pos--) {   // gradual sweep back
      myServo.write(pos);
      delay(10);
    }
    lastPosition = 0;    // Servo now at 0°
    actionDone = true;
    Serial.println("Servo now at 0°");
  }

  // Reset actionDone when sensors are released (all HIGH)
  if (sensor0 == HIGH && sensor90 == HIGH && sensor180 == HIGH) {
    actionDone = false;
  }

  delay(50);
}
