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

// LEDs
#define LED_A 0
#define LED_B 1
#define LED_C 2
#define LED_D 3

// Buttons (PCF)
#define BTN_A 4
#define BTN_B 5
#define BTN_C 6
#define BTN_D 7

// 🔥 UPDATED SWITCH PINS
#define POWER_SW D7
#define RESULT_SW D3
#define BUZZER D8
#define TAMPER_SW D0

// Voting
int voteA=0, voteB=0, voteC=0, voteD=0;
bool voted[50];

// System state
bool systemON = false;
bool lastPowerState = HIGH;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);

  pinMode(POWER_SW, INPUT_PULLUP);
  pinMode(RESULT_SW, INPUT_PULLUP);
  pinMode(TAMPER_SW, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // PCF8574
  pcf.begin();
  for(int i=0;i<8;i++) pcf.write(i,HIGH);

  // Fingerprint
  mySerial.begin(57600);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    show("Sensor Error");
    while (1);
  }

  show("System OFF");
}

// ================= LOOP =================
void loop() {

  // 🚨 Tamper Detection
  if (digitalRead(TAMPER_SW) == LOW) {
    show("TAMPER ALERT!");
    while (digitalRead(TAMPER_SW) == LOW) {
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
    }
  }

  // 🔘 POWER SWITCH (FIXED STRONG LOGIC)
  bool currentState = digitalRead(POWER_SW);

  if (currentState == LOW && lastPowerState == HIGH) {
    delay(50);
    if (digitalRead(POWER_SW) == LOW) {

      systemON = !systemON;

      if (systemON) show("System ON");
      else show("System OFF");

      while (digitalRead(POWER_SW) == LOW); // wait release
    }
  }
  lastPowerState = currentState;

  if (!systemON) return;

  // 📊 RESULT BUTTON
  if (digitalRead(RESULT_SW) == LOW) {
    delay(50);
    if (digitalRead(RESULT_SW) == LOW) {
      showResult();
      while (digitalRead(RESULT_SW) == LOW);
    }
  }

  // 👆 FINGERPRINT
  int id = getFinger();

  if (id > 0) {

    if (voted[id]) {
      show("Already Voted");
      longBeep();
      delay(2000);
      return;
    }

    voteSystem(id);
  }
}

// ================= FINGERPRINT =================
int getFinger() {
  if (finger.getImage() != FINGERPRINT_OK) {
    show("Place Finger");
    return -1;
  }

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

// ================= CAST =================
void cast(int id, char c) {

  voted[id] = true;

  if(c=='A'){ voteA++; pcf.write(LED_A,LOW); show("Voted A"); }
  if(c=='B'){ voteB++; pcf.write(LED_B,LOW); show("Voted B"); }
  if(c=='C'){ voteC++; pcf.write(LED_C,LOW); show("Voted C"); }
  if(c=='D'){ voteD++; pcf.write(LED_D,LOW); show("Voted D"); }

  beep(2,100);

  delay(1000);
  for(int i=0;i<4;i++) pcf.write(i,HIGH);

  delay(1500);
}

// ================= RESULT =================
void showResult() {
  display.clearDisplay();
  display.setCursor(0,0);

  display.println("RESULT:");
  display.print("A: "); display.println(voteA);
  display.print("B: "); display.println(voteB);
  display.print("C: "); display.println(voteC);
  display.print("D: "); display.println(voteD);

  display.display();
}

// ================= BUTTON =================
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

// ================= BUZZER =================
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
  delay(800);
  digitalWrite(BUZZER,LOW);
}

// ================= DISPLAY =================
void show(String msg){
  display.clearDisplay();
  display.setCursor(0,25);
  display.println(msg);
  display.display();
}