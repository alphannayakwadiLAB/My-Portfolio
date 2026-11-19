#include <Servo.h>

// Pins
const int hall0 = 2;
const int hall90 = 3;
const int hall180 = 4;
const int entryIR = 5;
const int exitIR = 6;
const int gasSensor = A0;
const int buzzer = 10;
const int exhaust = 11;
const int emerLight = 12;
const int timer10Btn = 7;
const int timer20Btn = 8;
const int stopBtn = 13;

Servo regulator;
int humanCount = 0;
bool servoOn = false;
unsigned long timerStart = 0;
unsigned long timerDuration = 0;
bool timerActive = false;

void setup() {
  pinMode(hall0, INPUT_PULLUP);
  pinMode(hall90, INPUT_PULLUP);
  pinMode(hall180, INPUT_PULLUP);
  pinMode(entryIR, INPUT_PULLUP);
  pinMode(exitIR, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(exhaust, OUTPUT);
  pinMode(emerLight, OUTPUT);
  pinMode(timer10Btn, INPUT_PULLUP);
  pinMode(timer20Btn, INPUT_PULLUP);
  pinMode(stopBtn, INPUT_PULLUP);

  regulator.attach(9);
  regulator.write(0); // Servo off position
  Serial.begin(9600);
}

void loop() {
  checkKnob();
  checkCounter();
  checkTimerButtons();
  checkGas();
  handleTimer();
}

// ========== FUNCTIONS ==========

// Knob control with Hall sensors
void checkKnob() {
  if (digitalRead(hall0) == LOW) {
    setServo(false); // 0° OFF
  } else if (digitalRead(hall90) == LOW || digitalRead(hall180) == LOW) {
    setServo(true);  // 90° or 180° ON
  }
}

// Human entry/exit counter
void checkCounter() {
  static int lastEntry = HIGH, lastExit = HIGH;
  int e = digitalRead(entryIR);
  int x = digitalRead(exitIR);

  if (lastEntry == HIGH && e == LOW) humanCount++;
  if (lastExit == HIGH && x == LOW && humanCount > 0) humanCount--;

  lastEntry = e;
  lastExit = x;

  // Auto-off condition
  if (servoOn && humanCount == 0 && !timerActive) {
    timerStart = millis();
    timerDuration = 300000UL; // 5 minutes
    timerActive = true;
  }
}

// Manual timer buttons
void checkTimerButtons() {
  if (digitalRead(timer10Btn) == LOW) startTimer(600000UL); // 10 min
  if (digitalRead(timer20Btn) == LOW) startTimer(1200000UL); // 20 min
  if (digitalRead(stopBtn) == LOW) stopTimer();
}

// Gas leak check
void checkGas() {
  int gasVal = analogRead(gasSensor);
  if (gasVal > 400) { // adjust threshold
    setServo(false); // OFF regulator
    digitalWrite(buzzer, HIGH);
    digitalWrite(exhaust, HIGH);
    digitalWrite(emerLight, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
    digitalWrite(exhaust, LOW);
    digitalWrite(emerLight, LOW);
  }
}

// Timer handler
void handleTimer() {
  if (timerActive && (millis() - timerStart >= timerDuration)) {
    stopTimer();
    setServo(false); // turn regulator off
    beepBuzzer();
  }
}

// Helper: start timer
void startTimer(unsigned long duration) {
  timerActive = true;
  timerStart = millis();
  timerDuration = duration;
}

// Helper: stop timer
void stopTimer() {
  timerActive = false;
}

// Helper: set servo ON/OFF
void setServo(bool on) {
  if (on && !servoOn) {
    regulator.write(90); // ON position
    servoOn = true;
  } else if (!on && servoOn) {
    regulator.write(0);  // OFF position
    servoOn = false;
  }
}

// Helper: buzzer beep pattern
void beepBuzzer() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(200);
  }
}
