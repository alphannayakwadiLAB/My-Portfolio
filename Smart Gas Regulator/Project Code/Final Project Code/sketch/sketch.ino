           #include <Servo.h>
Servo myServo;

// ----- Pin definitions -----
const int hall0 = 2;    // Emergency knob 0°
const int hall90 = 3;   // Knob ON
const int hall180 = 4;  // Knob ON alternative
const int sensor1 = 5;  // side-by-side sensors
const int sensor2 = 6;
const int timer05Pin = 7;
const int timer10Pin = 8;
const int servoPin = 9;
const int buzzerPin = 10;
const int exhaustPin = 11;
const int led05 = 12;
const int led10 = 13;
const int mq5Pin = A0;

// ----- Variables -----
int peopleCount = 0;
int lastPosition = 0; // 0=servo0°, 1=servo180°
bool actionDone = false;

// Timer variables
bool mainTimerActive = false;
unsigned long mainStart = 0;
unsigned long mainDuration = 0;
const unsigned long FIVE_MIN = 5UL * 60UL * 1000UL;
const unsigned long TEN_MIN = 10UL * 60UL * 1000UL;
const unsigned long BUZZ_TIME = 20UL * 1000UL;
bool buzzerOn = false;

// Gas leak variables
bool gasMonitorActive = false;
bool gasLeakDetected = false;
unsigned long gasStartTime = 0;
const unsigned long GAS_WAIT_TIME = 60UL * 1000UL;
int gasThreshold = 400;

// 5-minute waiting timer
bool waitTimerActive = false;
unsigned long waitStart = 0;
const unsigned long WAIT_TIME = 1UL * 60UL * 1000UL;

// ----- Sensor sequence variables -----
bool s1_last = HIGH;
bool s2_last = HIGH;
int sensorSequence = 0; // 1 = s1 first, 2 = s2 first

// ----- Knob state memory -----
bool hall90_last = HIGH;
bool hall180_last = HIGH;

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  myServo.write(0); // start at 0°

  pinMode(hall0, INPUT_PULLUP);
  pinMode(hall90, INPUT_PULLUP);
  pinMode(hall180, INPUT_PULLUP);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(timer05Pin, INPUT_PULLUP);
  pinMode(timer10Pin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(exhaustPin, OUTPUT);
  pinMode(led05, OUTPUT);
  pinMode(led10, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(exhaustPin, LOW);
  digitalWrite(led05, LOW);
  digitalWrite(led10, LOW);

  Serial.println("System initialized. Servo at 0°, gas monitor idle.");
}

void loop() {
  unsigned long now = millis();

  // ----- Read sensors -----
  int s1 = digitalRead(sensor1);
  int s2 = digitalRead(sensor2);
  int sensor0 = digitalRead(hall0);
  int sensor90 = digitalRead(hall90);
  int sensor180 = digitalRead(hall180);
  int timer05State = digitalRead(timer05Pin);
  int timer10State = digitalRead(timer10Pin);
  int gasValue = analogRead(mq5Pin);

  // ----- Side-by-side sensor sequence logic -----
  if (s1_last == HIGH && s1 == LOW && sensorSequence == 0) sensorSequence = 1; // s1 first
  if (s2_last == HIGH && s2 == LOW && sensorSequence == 0) sensorSequence = 2; // s2 first

  if (sensorSequence == 1 && s2_last == HIGH && s2 == LOW) {
    peopleCount++;
    Serial.print("Person ENTERED → Count: "); Serial.println(peopleCount);
    sensorSequence = 0;
    waitTimerActive = false;
  }

  if (sensorSequence == 2 && s1_last == HIGH && s1 == LOW) {
    if (peopleCount > 0) peopleCount--;
    Serial.print("Person EXITED → Count: "); Serial.println(peopleCount);
    sensorSequence = 0;
    if (peopleCount == 0 && lastPosition == 1) {
      waitTimerActive = true;
      waitStart = now;
      Serial.println("No one inside → 1-min waiting timer started...");
    }
  }

  s1_last = s1;
  s2_last = s2;

  // ----- KNOB ON (only on HIGH->LOW change) -----
  if (lastPosition == 0 && !actionDone) {
    if ((hall90_last == HIGH && sensor90 == LOW) || (hall180_last == HIGH && sensor180 == LOW)) {
      Serial.println("Knob ON → Servo to 180°, Regulator ON");
      for (int pos = 0; pos <= 180; pos++) { myServo.write(pos); delay(10); }
      lastPosition = 1;
      actionDone = true;
      gasMonitorActive = true;
      if (peopleCount == 0) {
        waitTimerActive = true;
        waitStart = now;
        Serial.println("No one inside → 1-min waiting timer started...");
      }
    }
  }
  hall90_last = sensor90;
  hall180_last = sensor180;

  // ----- 5-min waiting timer -----
  if (waitTimerActive) {
    if (peopleCount > 0) {
      waitTimerActive = false;
      Serial.println("Someone entered → waiting timer cancelled.");
    } else if (now - waitStart >= WAIT_TIME) {
      Serial.println("1 minutes passed with no one inside → Servo OFF.");
      turnOffRegulator();
      waitTimerActive = false;
    }
  }

  // ----- Timer start -----
  if (lastPosition == 1 && !mainTimerActive) {
    if (timer05State == LOW) startMainTimer(FIVE_MIN, led05);
    if (timer10State == LOW) startMainTimer(TEN_MIN, led10);
  }

  // ----- Main Timer Running -----
  if (mainTimerActive) {
    unsigned long elapsed = now - mainStart;
    if (mainDuration == FIVE_MIN) digitalWrite(led05, HIGH);
    if (mainDuration == TEN_MIN) digitalWrite(led10, HIGH);

    if (elapsed >= mainDuration - BUZZ_TIME && elapsed < mainDuration) {
      digitalWrite(buzzerPin, HIGH);
      if (!buzzerOn) { Serial.println("Last 20 sec → buzzer ON."); buzzerOn = true; }
    } else {
      digitalWrite(buzzerPin, LOW); // ensure buzzer OFF outside last 20s
    }

    if (elapsed >= mainDuration) {
      Serial.println("Main timer complete → Servo OFF.");
      turnOffRegulator();    
      mainTimerActive = false;
      buzzerOn = false;
      digitalWrite(led05, LOW);
      digitalWrite(led10, LOW);
      digitalWrite(buzzerPin, LOW);
    }
  }

  // ----- Gas Leak Detection -----
  if (gasMonitorActive && lastPosition == 1) {
    if (gasValue > gasThreshold && !gasLeakDetected) {
      Serial.println("Gas leak detected! Turning off regulator & starting exhaust.");
      gasLeakDetected = true;
      gasStartTime = now;
      turnOffRegulator();
      digitalWrite(exhaustPin, HIGH);
      mainTimerActive = false;
      digitalWrite(led05, LOW);
      digitalWrite(led10, LOW);
      digitalWrite(buzzerPin, LOW);
    }

    if (gasLeakDetected) {
      if (gasValue <= gasThreshold - 50) {
        Serial.println(" Gas cleared. Exhaust OFF.");
        digitalWrite(exhaustPin, LOW);
        digitalWrite(buzzerPin, LOW);
        gasLeakDetected = false;
      } else if (now - gasStartTime >= GAS_WAIT_TIME) {
        digitalWrite(buzzerPin, HIGH);
        Serial.println(" Gas level high >1min → buzzer ON!");
      }
    }
  }

  // ----- EMERGENCY OFF (only if hall0 pressed) -----
  if (sensor0 == LOW && lastPosition == 1) {
    Serial.println("Emergency OFF → Everything reset.");
    turnOffRegulator();
    mainTimerActive = false;
    gasMonitorActive = false;
    gasLeakDetected = false;
    waitTimerActive = false;
    digitalWrite(buzzerPin, LOW);
    digitalWrite(exhaustPin, LOW);
    digitalWrite(led05, LOW);
    digitalWrite(led10, LOW);
  }

  if (sensor0 == HIGH && sensor90 == HIGH && sensor180 == HIGH) actionDone = false;

  delay(50);
}

// ----- Helper functions -----
void turnOffRegulator() {
  for (int pos = 180; pos >= 0; pos--) { myServo.write(pos); delay(10); }
  lastPosition = 0;
  Serial.println("Servo OFF (0°).");
  digitalWrite(buzzerPin, LOW);
}

void startMainTimer(unsigned long duration, int ledPin) {
  mainTimerActive = true;
  mainStart = millis();
  mainDuration = duration;
  digitalWrite(led05, LOW);
  digitalWrite(led10, LOW);
  digitalWrite(ledPin, HIGH);
  buzzerOn = false;
  digitalWrite(buzzerPin, LOW);
  Serial.print("Timer started for ");
  Serial.print(duration / 60000);
  Serial.println(" minutes");
}

