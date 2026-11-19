#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -------- Pin Definitions --------
#define BTN_4H   14   // D5
#define BTN_5H   12   // D6
#define BTN_6H   13   // D7
#define BTN_ON   2    // D4
#define BTN_OFF  0    // D3
#define RELAY    15   // D8 (Active LOW relay assumed)

// -------- Timer Variables --------
unsigned long startTime = 0;
unsigned long duration = 0;
bool timerActive = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

void showRemainingTime() {
  unsigned long remaining = (duration - (millis() - startTime)) / 1000;

  int hours = remaining / 3600;
  int minutes = (remaining % 3600) / 60;
  int seconds = remaining % 60;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("ALPHAN NAYAKWADI");

  display.setTextSize(2);
  display.setCursor(0,20);

  if(hours < 10) display.print("0");
  display.print(hours);
  display.print(":");

  if(minutes < 10) display.print("0");
  display.print(minutes);
  display.print(":");

  if(seconds < 10) display.print("0");
  display.print(seconds);

  display.display();
}

void showMessage(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);
  display.println("ALPHAN NAYAKWADI");
  display.setCursor(0,30);
  display.println(line1);
  display.setCursor(0,45);
  display.println(line2);
  display.display();
}

void setup() {
  Serial.begin(9600);

  pinMode(BTN_4H, INPUT_PULLUP);
  pinMode(BTN_5H, INPUT_PULLUP);
  pinMode(BTN_6H, INPUT_PULLUP);
  pinMode(BTN_ON, INPUT_PULLUP);
  pinMode(BTN_OFF, INPUT_PULLUP);
  pinMode(RELAY, OUTPUT);

  digitalWrite(RELAY, HIGH);  // OFF (Active LOW relay)

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while(true);
  }

  showMessage("Fan Controller", "System Ready");
}

void loop() {

  // -------- Button Handling --------
  if (millis() - lastDebounceTime > debounceDelay) {

    if (digitalRead(BTN_4H) == LOW) {
      duration = 4UL * 60 * 60 * 1000;
      startTime = millis();
      timerActive = true;
      digitalWrite(RELAY, LOW);   // ON
      showMessage("Mode: 4 Hours", "Fan ON");
      lastDebounceTime = millis();
    }

    else if (digitalRead(BTN_5H) == LOW) {
      duration = 5UL * 60 * 60 * 1000;
      startTime = millis();
      timerActive = true;
      digitalWrite(RELAY, LOW);
      showMessage("Mode: 5 Hours", "Fan ON");
      lastDebounceTime = millis();
    }

    else if (digitalRead(BTN_6H) == LOW) {
      duration = 6UL * 60 * 60 * 1000;
      startTime = millis();
      timerActive = true;
      digitalWrite(RELAY, LOW);
      showMessage("Mode: 6 Hours", "Fan ON");
      lastDebounceTime = millis();
    }

    else if (digitalRead(BTN_ON) == LOW) {
      timerActive = false;
      digitalWrite(RELAY, LOW);
      showMessage("Continuous Mode", "Fan ON");
      lastDebounceTime = millis();
    }

    else if (digitalRead(BTN_OFF) == LOW) {
      timerActive = false;
      digitalWrite(RELAY, HIGH);
      showMessage("Fan OFF", "Manual Stop");
      lastDebounceTime = millis();
    }
  }

  // -------- Timer Running --------
  if (timerActive) {
    showRemainingTime();
  }

  // -------- Timer Finish --------
  if (timerActive && (millis() - startTime >= duration)) {
    digitalWrite(RELAY, HIGH);  // OFF
    timerActive = false;
    showMessage("Timer Finished", "Fan OFF");
  }
}