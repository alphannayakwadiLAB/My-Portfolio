#include <Servo.h>

Servo myServo;

// Hall sensor pins
const int hall0 = 2;
const int hall90 = 3;
const int hall180 = 4;

// Obstacle sensor pins
const int sensorIn = 5;
const int sensorOut = 6;

// Variables
int peopleCount = 0;
int lastPosition = 0;  // 0 = servo at 0°, 1 = servo at 180°
bool actionDone = false;
bool timerStarted = false;
unsigned long timerStart = 0;
const unsigned long TIME_LIMIT = 5UL * 60UL * 1000UL; // 5 minutes in milliseconds

void setup() {
  Serial.begin(9600);
  myServo.attach(9);
  myServo.write(0);

  pinMode(hall0, INPUT);
  pinMode(hall90, INPUT);
  pinMode(hall180, INPUT);
  pinMode(sensorIn, INPUT);
  pinMode(sensorOut, INPUT);

  Serial.println("System started. Servo at 0°, waiting for knob action...");
}

void loop() {
  // Read sensors
  int sensor0 = digitalRead(hall0);
  int sensor90 = digitalRead(hall90);
  int sensor180 = digitalRead(hall180);
  int inState = digitalRead(sensorIn);
  int outState = digitalRead(sensorOut);

  // --- People counter logic ---
  static bool inPrev = HIGH;
  static bool outPrev = HIGH;

  if (inPrev == HIGH && inState == LOW) {
    peopleCount++;
    Serial.print("Person entered. Count = ");
    Serial.println(peopleCount);
  }

  if (outPrev == HIGH && outState == LOW && peopleCount > 0) {
    peopleCount--;
    Serial.print("Person exited. Count = ");
    Serial.println(peopleCount);
  }

  inPrev = inState;
  outPrev = outState;

  // --- Knob ON logic ---
  if (lastPosition == 0 && (sensor90 == LOW || sensor180 == LOW) && !actionDone) {
    Serial.println("Knob ON detected → Servo rotating to 180°...");
    for (int pos = 0; pos <= 180; pos++) {
      myServo.write(pos);
      delay(10);
    }
    lastPosition = 1;
    actionDone = true;
    Serial.println("Servo ON (180°)");

    if (peopleCount == 0) {
      timerStarted = true;
      timerStart = millis();
      Serial.println("No one inside. 5-minute timer started...");
    }
  }

  // --- Timer check ---
  if (timerStarted && peopleCount > 0) {
    timerStarted = false;
    Serial.println("Person detected before timeout → Regulator stays ON.");
  }

  if (timerStarted && (millis() - timerStart >= TIME_LIMIT) && peopleCount == 0) {
    Serial.println("No entry for 5 minutes → Regulator turning OFF...");
    for (int pos = 180; pos >= 0; pos--) {
      myServo.write(pos);
      delay(10);
    }
    lastPosition = 0;
    timerStarted = false;
    actionDone = true;
  }

  // --- Knob OFF logic ---
  if (lastPosition == 1 && sensor0 == LOW && !actionDone) {
    Serial.println("Knob OFF detected → Servo rotating to 0°...");
    for (int pos = 180; pos >= 0; pos--) {
      myServo.write(pos);
      delay(10);
    }
    lastPosition = 0;
    actionDone = true;
    timerStarted = false;
    Serial.println("Servo OFF (0°). System reset.");
  }

  // --- Reset actionDone when all sensors released ---
  if (sensor0 == HIGH && sensor90 == HIGH && sensor180 == HIGH) {
    actionDone = false;
  }

  delay(50);
}
