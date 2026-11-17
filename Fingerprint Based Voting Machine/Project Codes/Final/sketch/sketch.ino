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
bool voted[50];

// System
bool systemON = false;
bool lastPowerState = HIGH;

// Admin Logic
int failCount = 0;
int adminAccess = 0;
bool adminMode = false;

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

// ================= MEMORY =================
void memoryCheck() {
  display.clearDisplay();
  display.setCursor(0,10);
  display.println("Clear Memory?");
  display.setCursor(0,30);
  display.println("RESULT = YES");
  display.setCursor(0,45);
  display.println("POWER = NO");
  display.display();

  while (true) {
    if (digitalRead(RESULT_SW) == LOW) {
      voteA = voteB = voteC = voteD = 0;
      writeEEPROM(0,0);
      writeEEPROM(2,0);
      writeEEPROM(4,0);
      writeEEPROM(6,0);
      show("Memory Cleared");
      beep(2,200);
      delay(2000);
      break;
    }
    if (digitalRead(POWER_SW) == LOW) {
      show("Memory Kept");
      beep(1,200);
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

  // POWER TOGGLE
  bool currentState = digitalRead(POWER_SW);
  if (currentState == LOW && lastPowerState == HIGH) {
    delay(50);
    if (digitalRead(POWER_SW) == LOW) {
      systemON = !systemON;
      show(systemON ? "System ON" : "System OFF");
      while (digitalRead(POWER_SW) == LOW);
    }
  }
  lastPowerState = currentState;

  if (!systemON) return;

  // RESULT BUTTON
  if (digitalRead(RESULT_SW) == LOW) {
    delay(50);
    if (digitalRead(RESULT_SW) == LOW) {
      showResult();
      while (digitalRead(RESULT_SW) == LOW);
    }
  }

  int id = getFinger();

  if (id == -2) return;

  if (id == -1) {
    failCount++;
    if (failCount >= 10) {
      show("Admin Required");
      adminMode = true;
      failCount = 0;
      delay(1500);
    }
    return;
  }

  // ADMIN MODE
  if (adminMode) {
    if (id == 1 || id == 6) {
      adminAccess++;
      show("Admin Verified");
      beep(2,200);
      adminMode = false;
      delay(1000);
      voteSystem(id);
    } else {
      show("Invalid Admin");
      longBeep();
      delay(1000);
    }
    return;
  }

  // NORMAL
  failCount = 0;

  if (voted[id]) {
    show("Already Voted");
    longBeep();
    delay(1500);
    return;
  }

  voteSystem(id);
}

// ================= RESULT =================
void showResult() {

  display.setTextSize(2);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("A");
  display.setCursor(0,30);
  display.print("Votes:");
  display.println(voteA);
  display.display();
  delay(4000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("B");
  display.setCursor(0,30);
  display.print("Votes:");
  display.println(voteB);
  display.display();
  delay(4000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("C");
  display.setCursor(0,30);
  display.print("Votes:");
  display.println(voteC);
  display.display();
  delay(4000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("D");
  display.setCursor(0,30);
  display.print("Votes:");
  display.println(voteD);
  display.display();
  delay(4000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Admin Access:");
  display.setCursor(0,40);
  display.print(adminAccess);
  display.display();
  delay(4000);

  showWinner();
}

// ================= WINNER =================
void showWinner() {

  int maxVotes = voteA;
  String winner = "A";

  if (voteB > maxVotes) { maxVotes = voteB; winner = "B"; }
  if (voteC > maxVotes) { maxVotes = voteC; winner = "C"; }
  if (voteD > maxVotes) { maxVotes = voteD; winner = "D"; }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("WINNER");
  display.setCursor(0,25);
  display.print(winner);

  display.setTextSize(1);
  display.setCursor(0,55);
  display.print("Votes: ");
  display.println(maxVotes);

  display.display();
  beep(3,150);
  delay(5000);
}

// ================= FINGER =================
int getFinger() {

  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER) {
    show("Place Finger");
    delay(400);
    return -2;
  }

  if (p != FINGERPRINT_OK) return -1;

  beep(1,100);
  finger.image2Tz();

  if (finger.fingerFastSearch() != FINGERPRINT_OK) {
    show("Not Found");
    beep(3,80);
    delay(1500);
    return -1;
  }

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
    if (btn(BTN_A)) { cast(id,'A'); break; }
    if (btn(BTN_B)) { cast(id,'B'); break; }
    if (btn(BTN_C)) { cast(id,'C'); break; }
    if (btn(BTN_D)) { cast(id,'D'); break; }
  }
}

void cast(int id, char c) {

  voted[id] = true;

  if(c=='A'){ voteA++; writeEEPROM(0,voteA); pcf.write(LED_A,LOW); show("Voted A"); }
  if(c=='B'){ voteB++; writeEEPROM(2,voteB); pcf.write(LED_B,LOW); show("Voted B"); }
  if(c=='C'){ voteC++; writeEEPROM(4,voteC); pcf.write(LED_C,LOW); show("Voted C"); }
  if(c=='D'){ voteD++; writeEEPROM(6,voteD); pcf.write(LED_D,LOW); show("Voted D"); }

  beep(2,100);
  delay(1000);

  for(int i=0;i<4;i++) pcf.write(i,HIGH);

  delay(1000);
}

// ================= UTIL =================
bool btn(int pin) {
  if (pcf.read(pin)==LOW) {
    delay(50);
    if (pcf.read(pin)==LOW) {
      while(pcf.read(pin)==LOW);
      return true;
    }
  }
  return false;
}

void beep(int t,int d){
  for(int i=0;i<t;i++){
    digitalWrite(BUZZER,HIGH);
    delay(d);
    digitalWrite(BUZZER,LOW);
    delay(d);
  }
}

void longBeep(){
  digitalWrite(BUZZER,HIGH);
  delay(700);
  digitalWrite(BUZZER,LOW);
}

void show(String msg){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,25);
  display.println(msg);
  display.display();
}