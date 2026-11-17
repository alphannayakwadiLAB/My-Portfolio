#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <PCF8574.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= Fingerprint =================
SoftwareSerial mySerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// ================= PCF8574 =================
PCF8574 pcf(0x20);   // CHANGE TO 0x27 if not working

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

bool voted[50];

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // I2C
  Wire.begin(D2, D1);

  // PCF8574
  pcf.begin();

  for (int i = 0; i < 4; i++) {
    pcf.pinMode(i, OUTPUT);
    pcf.digitalWrite(i, LOW);
  }

  for (int i = 4; i < 8; i++) {
    pcf.pinMode(i, INPUT);
  }

  // Fingerprint
  mySerial.begin(57600);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    show("Sensor Error");
    while (1);
  }

  show("System Ready");
  delay(2000);
}

// ================= LOOP =================
void loop() {
  int id = getFinger();

  if (id > 0) {

    if (voted[id]) {
      show("Already Voted");
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

  finger.image2Tz();

  if (finger.fingerFastSearch() != FINGERPRINT_OK) {
    show("Not Found");
    delay(1500);
    return -1;
  }

  int id = finger.fingerID;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("User ID: ");
  display.println(id);
  display.display();

  delay(1500);
  return id;
}

// ================= VOTING =================
void voteSystem(int id) {

  show("Press A B C D");

  while (true) {

    if (btn(BTN_A)) {
      cast(id, 'A');
      break;
    }

    if (btn(BTN_B)) {
      cast(id, 'B');
      break;
    }

    if (btn(BTN_C)) {
      cast(id, 'C');
      break;
    }

    if (btn(BTN_D)) {
      cast(id, 'D');
      break;
    }
  }
}

// ================= CAST VOTE =================
void cast(int id, char c) {

  voted[id] = true;

  display.clearDisplay();
  display.setCursor(0, 20);

  if (c == 'A') {
    pcf.digitalWrite(LED_A, HIGH);
    show("Voted A");
  }
  else if (c == 'B') {
    pcf.digitalWrite(LED_B, HIGH);
    show("Voted B");
  }
  else if (c == 'C') {
    pcf.digitalWrite(LED_C, HIGH);
    show("Voted C");
  }
  else if (c == 'D') {
    pcf.digitalWrite(LED_D, HIGH);
    show("Voted D");
  }

  delay(1000);

  // OFF LEDs
  for (int i = 0; i < 4; i++) {
    pcf.digitalWrite(i, LOW);
  }

  delay(1500);
}

// ================= BUTTON =================
bool btn(int pin) {
  if (pcf.digitalRead(pin) == LOW) {
    delay(50);
    if (pcf.digitalRead(pin) == LOW) {
      while (pcf.digitalRead(pin) == LOW);
      return true;
    }
  }
  return false;
}

// ================= OLED =================
void show(String msg) {
  display.clearDisplay();
  display.setCursor(0, 25);
  display.println(msg);
  display.display();
}