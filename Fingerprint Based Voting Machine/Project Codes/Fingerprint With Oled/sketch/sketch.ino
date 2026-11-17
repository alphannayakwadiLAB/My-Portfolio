#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include "RTClib.h"

// OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Fingerprint
SoftwareSerial mySerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// RTC
RTC_DS3231 rtc;

// EEPROM
#define EEPROM_ADDR 0x57

// Buttons
#define CAND_A D3
#define CAND_B D4
#define CAND_C D7
#define CAND_D D0
#define RESULT_BTN D8

// Output
#define LED 1
#define BUZZER 3

// Votes
int votesA = 0, votesB = 0, votesC = 0, votesD = 0;

bool voted = false;
int addr = 10;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(CAND_A, INPUT_PULLUP);
  pinMode(CAND_B, INPUT_PULLUP);
  pinMode(CAND_C, INPUT_PULLUP);
  pinMode(CAND_D, INPUT_PULLUP);
  pinMode(RESULT_BTN, INPUT_PULLUP);

  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  rtc.begin();

  mySerial.begin(57600);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    show("FP ERROR");
    while (1);
  }

  show("READY");
}

// ---------------- LOOP ----------------
void loop() {

  // RESULT BUTTON (ANYTIME)
  if (buttonPressed(RESULT_BTN)) {
    showResultScreen();
    delay(3000);
  }

  int id = getFingerprintID();
  if (id == -1) return;

  voted = false;
  show("Select Candidate");

  while (!voted) {

    if (buttonPressed(CAND_A)) {
      votesA++;
      voteAction("A", 1);
    }

    if (buttonPressed(CAND_B)) {
      votesB++;
      voteAction("B", 2);
    }

    if (buttonPressed(CAND_C)) {
      votesC++;
      voteAction("C", 3);
    }

    if (buttonPressed(CAND_D)) {
      votesD++;
      voteAction("D", 4);
    }
  }
}

// ---------------- VOTE ACTION ----------------
void voteAction(String name, byte id) {
  storeVote(id);

  digitalWrite(LED, HIGH);
  digitalWrite(BUZZER, HIGH);

  show("Voted " + name);

  delay(500);

  digitalWrite(LED, LOW);
  digitalWrite(BUZZER, LOW);

  voted = true;
  delay(1000);
}

// ---------------- RESULT DISPLAY ----------------
void showResultScreen() {
  display.clearDisplay();

  display.setCursor(0,0);
  display.println("RESULT");

  display.setCursor(0,15);
  display.print("A: "); display.println(votesA);

  display.setCursor(0,25);
  display.print("B: "); display.println(votesB);

  display.setCursor(0,35);
  display.print("C: "); display.println(votesC);

  display.setCursor(0,45);
  display.print("D: "); display.println(votesD);

  display.display();
}

// ---------------- BUTTON ----------------
bool buttonPressed(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(50);
    if (digitalRead(pin) == LOW) {
      while (digitalRead(pin) == LOW);
      return true;
    }
  }
  return false;
}

// ---------------- DISPLAY ----------------
void show(String msg) {
  display.clearDisplay();
  display.setCursor(0, 20);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(msg);
  display.display();
}

// ---------------- FINGERPRINT ----------------
int getFingerprintID() {
  if (finger.getImage() != FINGERPRINT_OK) {
    show("Place Finger");
    return -1;
  }

  if (finger.image2Tz() != FINGERPRINT_OK) return -1;

  if (finger.fingerFastSearch() != FINGERPRINT_OK) {
    show("Not Found");
    delay(1000);
    return -1;
  }

  show("Access OK");
  delay(1000);
  return finger.fingerID;
}

// ---------------- STORE VOTE ----------------
void storeVote(byte candidate) {
  DateTime now = rtc.now();

  writeEEPROM(addr++, candidate);
  writeEEPROM(addr++, now.hour());
  writeEEPROM(addr++, now.minute());
  writeEEPROM(addr++, now.second());
}

// ---------------- EEPROM ----------------
void writeEEPROM(int addr, byte data) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(addr >> 8);
  Wire.write(addr & 0xFF);
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}