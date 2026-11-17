#include <Servo.h>

Servo myServo;

// ----- Pin definitions -----
const int hall0 = 2;
const int hall90 = 3;
const int hall180 = 4;
const int sensorIn = 5;
const int sensorOut = 6;
const int timer10Pin = 7;
const int timer20Pin = 8;
const int servoPin = 9;
const int buzzerPin = 10;

// ----- Variables -----
int peopleCount = 0;
int lastPosition = 0; // 0=servo0°, 1=servo180°
bool actionDone = false;

// 5-minute waiting timer
bool waitTimerActive = false;
unsigned long waitStart = 0;
const unsigned long WAIT_TIME = 5UL * 60UL * 1000UL;

// Main timer (10min / 20min)
bool mainTimerActive = false;
unsigned long mainStart = 0;
unsigned long mainDuration = 0;
const unsigned long TEN_MIN = 10UL * 60UL * 1000UL;
const unsigned long TWENTY_MIN = 20UL * 60UL * 1000UL;
const unsigned long BUZZ_TIME = 20UL * 1000UL; // last 20s
bool buzzerOn = false;

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  myServo.write(0);

  pinMode(hall0, INPUT);
  pinMode(hall90, INPUT);
  pinMode(hall180, INPUT);
  pinMode(sensorIn, INPUT);
  pinMode(sensorOut, INPUT);
  pinMode(timer10Pin, INPUT);
  pinMode(timer20Pin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  Serial.println("System ready. Servo at 0°.");
}

void loop() {
  // ----- Read sensors -----
  int sensor0 = digitalRead(hall0);
  int sensor90 = digitalRead(hall90);
  int sensor180 = digitalRead(hall180);
  int inState = digitalRead(sensorIn);
  int outState = digitalRead(sensorOut);
  int timer10State = digitalRead(timer10Pin);
  int timer20State = digitalRead(timer20Pin);

  // ----- People counter -----
  static bool inPrev = HIGH, outPrev = HIGH;

  if (inPrev == HIGH && inState == LOW) {
    peopleCount++;
    Serial.print("Person entered. Count=");
    Serial.println(peopleCount);
  }
  if (outPrev == HIGH && outState == LOW && peopleCount > 0) {
    peopleCount--;
    Serial.print("Person exited. Count=");
    Serial.println(peopleCount);
  }
  inPrev = inState;
  outPrev = outState;

  // ----- KNOB ON -----
  if (lastPosition == 0 && (sensor90 == LOW || sensor180 == LOW) && !actionDone) {
    Serial.println("Knob ON → Servo to 180°");
    for (int pos = 0; pos <= 180; pos++) { myServo.write(pos); delay(10); }
    lastPosition = 1;
    actionDone = true;

    // Start 5-min wait if no one inside
    if (peopleCount == 0) {
      waitTimerActive = true;
      waitStart = millis();
      Serial.println("No one inside. 5-minute waiting timer started...");
    } else {
      waitTimerActive = false;
    }
  }

  // ----- 5-minute waiting timer -----
  if (waitTimerActive) {
    if (peopleCount > 0) {
      waitTimerActive = false;
      Serial.println("Person entered → waiting timer cancelled.");
    } else if (millis() - waitStart >= WAIT_TIME) {
      Serial.println("5 minutes passed with no one inside → Servo OFF.");
      turnOffRegulator();
      waitTimerActive = false;
    }
  }

  // ----- Timer switches -----
  if (lastPosition == 1 && !mainTimerActive) {
    if (timer10State == LOW) {
      mainTimerActive = true;
      mainStart = millis();
      mainDuration = TEN_MIN;
      Serial.println("10-minute timer started.");
    }
    if (timer20State == LOW) {
      mainTimerActive = true;
      mainStart = millis();
      mainDuration = TWENTY_MIN;
      Serial.println("20-minute timer started.");
    }
  }

  // ----- Main timer running -----
  if (mainTimerActive) {
    unsigned long elapsed = millis() - mainStart;
    unsigned long remaining = (mainDuration > elapsed) ? (mainDuration - elapsed) : 0;

    if (remaining <= BUZZ_TIME && remaining > 0) {
      digitalWrite(buzzerPin, HIGH);
      if (!buzzerOn) {
        Serial.println("Last 20 seconds → buzzer ON.");
        buzzerOn = true;
      }
    } else {
      digitalWrite(buzzerPin, LOW);
    }

    if (elapsed >= mainDuration) {
      Serial.println("Main timer complete → Servo OFF.");
      digitalWrite(buzzerPin, LOW);
      turnOffRegulator();
      mainTimerActive = false;
      buzzerOn = false;
    }
  }

  // ----- EMERGENCY OFF (Knob 0°) -----
  if (lastPosition == 1 && sensor0 == LOW) {
    Serial.println("Emergency OFF → Everything reset.");
    turnOffRegulator();
    waitTimerActive = false;
    mainTimerActive = false;
    digitalWrite(buzzerPin, LOW);
    buzzerOn = false;
  }

  // ----- Reset actionDone when knob released -----
  if (sensor0 == HIGH && sensor90 == HIGH && sensor180 == HIGH) {
    actionDone = false;
  }

  delay(50);
}

// ----- Helper: turn off regulator -----
void turnOffRegulator() {
  for (int pos = 180; pos >= 0; pos--) {
    myServo.write(pos);
    delay(10);
  }
  lastPosition = 0;
  Serial.println("Servo OFF (0°).");
}
