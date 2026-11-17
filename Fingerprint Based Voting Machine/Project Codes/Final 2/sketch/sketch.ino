#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <PCF8574.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Fingerprint
SoftwareSerial mySerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// PCF8574
PCF8574 pcf(0x20);

// EEPROM
#define EEPROM_ADDR 0x57

// LEDs
#define LED_A 0
#define LED_B 1
#define LED_C 2
#define LED_D 3

// Buttons
#define BTN_A 4
#define BTN_B 5
#define BTN_C 6
#define BTN_D 7

// Switches
#define POWER_SW D7
#define RESULT_SW D3
#define BUZZER D8
#define TAMPER_SW D0

// Votes
int voteA=0, voteB=0, voteC=0, voteD=0;
int officerA=0, officerB=0, officerC=0, officerD=0;

bool voted[50];

// System
bool systemON = false;
bool lastPowerState = HIGH;
int failCount = 0;

// ================= EEPROM =================
void writeEEPROM(int addr, int value) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((addr >> 8) & 0xFF);
  Wire.write(addr & 0xFF);
  Wire.write((value >> 8) & 0xFF);
  Wire.write(value & 0xFF);
  Wire.endTransmission();
  delay(5);
}

int readEEPROM(int addr) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((addr >> 8) & 0xFF);
  Wire.write(addr & 0xFF);
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_ADDR, 2);
  return (Wire.read() << 8) | Wire.read();
}

// ================= MEMORY CHECK =================
void memoryCheck() {

  display.clearDisplay();
  display.setCursor(0,10);
  display.println("Clear Memory?");
  display.println("RESULT = YES");
  display.println("POWER = NO");
  display.display();

  while (true) {

    if (digitalRead(RESULT_SW) == LOW) {
      voteA=voteB=voteC=voteD=0;
      writeEEPROM(0,0); writeEEPROM(2,0);
      writeEEPROM(4,0); writeEEPROM(6,0);
      show("Memory Cleared");
      delay(2000);
      break;
    }

    if (digitalRead(POWER_SW) == LOW) {
      show("Memory Kept");
      delay(2000);
      break;
    }
  }
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);
  Wire.begin(D2, D1);

  pinMode(POWER_SW, INPUT_PULLUP);
  pinMode(RESULT_SW, INPUT_PULLUP);
  pinMode(TAMPER_SW, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);

  pcf.begin();
  for(int i=0;i<8;i++) pcf.write(i,HIGH);

  mySerial.begin(57600);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    show("Sensor Error");
    while (1);
  }

  voteA = readEEPROM(0);
  voteB = readEEPROM(2);
  voteC = readEEPROM(4);
  voteD = readEEPROM(6);

  memoryCheck();

  show("System OFF");
}

// ================= LOOP =================
void loop() {

  // Tamper
  if (digitalRead(TAMPER_SW) == LOW) {
    while (digitalRead(TAMPER_SW) == LOW) {
      digitalWrite(BUZZER,HIGH); delay(200);
      digitalWrite(BUZZER,LOW); delay(200);
    }
  }

  // Power toggle
  bool currentState = digitalRead(POWER_SW);
  if (currentState == LOW && lastPowerState == HIGH) {
    delay(50);
    systemON = !systemON;
    show(systemON ? "System ON" : "System OFF");
    while (digitalRead(POWER_SW)==LOW);
  }
  lastPowerState = currentState;

  if (!systemON) return;

  // Result button
  if (digitalRead(RESULT_SW) == LOW) {
    showResult();
    while (digitalRead(RESULT_SW)==LOW);
  }

  int id = getFinger();

  if (id > 0) {

    if (voted[id]) {
      show("Already Voted");
      delay(2000);
      return;
    }

    voteSystem(id);
  }

  delay(200); // stability
}

// ================= OFFICER MODE =================
void officerMode() {

  show("Officer Mode");
  delay(1500);

  while (true) {
    if (btn(BTN_A)) { officerA++; break; }
    if (btn(BTN_B)) { officerB++; break; }
    if (btn(BTN_C)) { officerC++; break; }
    if (btn(BTN_D)) { officerD++; break; }
  }

  beep(2,100);
  show("Manual Vote Done");
  delay(1500);
}

// ================= FIXED FINGER FUNCTION =================
int getFinger() {

  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER) return -1;
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();

  if (p != FINGERPRINT_OK) {

    show("Not Found");
    beep(2,80);

    failCount++;

    delay(1500);

    if (failCount >= 3) {
      officerMode();
      failCount = 0;
    }

    return -1;
  }

  failCount = 0;

  int id = finger.fingerID;

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("ID: ");
  display.println(id);
  display.display();

  delay(1000);

  return id;
}

// ================= VOTING =================
void voteSystem(int id) {

  show("Press A B C D");

  while (true) {
    if (btn(BTN_A)) { voteA++; writeEEPROM(0,voteA); break; }
    if (btn(BTN_B)) { voteB++; writeEEPROM(2,voteB); break; }
    if (btn(BTN_C)) { voteC++; writeEEPROM(4,voteC); break; }
    if (btn(BTN_D)) { voteD++; writeEEPROM(6,voteD); break; }
  }

  voted[id] = true;
  beep(2,100);
}

// ================= RESULT =================
void showResult() {

  display.setTextSize(2);

  showCandidate("A", voteA+officerA);
  showCandidate("B", voteB+officerB);
  showCandidate("C", voteC+officerC);
  showCandidate("D", voteD+officerD);

  showWinner();
}

void showCandidate(String name, int votes){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(name);
  display.setCursor(0,30);
  display.print("Votes:");
  display.println(votes);
  display.display();
  delay(30000);
}

// ================= WINNER =================
void showWinner() {

  int A = voteA+officerA;
  int B = voteB+officerB;
  int C = voteC+officerC;
  int D = voteD+officerD;

  int maxVotes = A;
  String winner = "A";

  if (B>maxVotes){ maxVotes=B; winner="B"; }
  if (C>maxVotes){ maxVotes=C; winner="C"; }
  if (D>maxVotes){ maxVotes=D; winner="D"; }

  display.clearDisplay();
  display.setTextSize(2);
  display.println("WINNER");
  display.println(winner);

  display.setTextSize(1);
  display.print("Votes:");
  display.println(maxVotes);
  display.display();

  delay(5000);
}

// ================= UTIL =================
bool btn(int pin){
  if (pcf.read(pin)==LOW){
    delay(50);
    while(pcf.read(pin)==LOW);
    return true;
  }
  return false;
}

void beep(int t,int d){
  for(int i=0;i<t;i++){
    digitalWrite(BUZZER,HIGH); delay(d);
    digitalWrite(BUZZER,LOW); delay(d);
  }
}

void show(String msg){
  display.clearDisplay();
  display.setCursor(0,25);
  display.println(msg);
  display.display();
}