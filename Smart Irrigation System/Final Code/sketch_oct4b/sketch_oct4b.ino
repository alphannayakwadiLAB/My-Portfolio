#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins
#define TEMP_PIN A0
#define VIBRATION_PIN D4
#define RELAY_PIN D3
#define BUZZER_PIN D8

int tempValue = 0;
float temperature = 0;
int vibrationState = 0;

float tempThreshold = 40.0;  // Adjust if needed

void setup() {
  Serial.begin(115200);

  pinMode(VIBRATION_PIN, INPUT_PULLUP);  // Stable input
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);   // Relay ON (Active LOW)
  digitalWrite(BUZZER_PIN, LOW);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Industrial System");
  display.println("Starting...");
  display.display();
  delay(2000);
}

void loop() {

  // 🔹 Read LM35
  tempValue = analogRead(TEMP_PIN);
  float voltage = tempValue * (3.3 / 1023.0);
  temperature = voltage * 100.0;

  // 🔹 Read vibration
  vibrationState = digitalRead(VIBRATION_PIN);

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C | Vib: ");
  Serial.println(vibrationState);

  display.clearDisplay();
  display.setCursor(0, 0);

  // 🔥 Fault Logic
  if (temperature > tempThreshold || vibrationState == LOW) {

    digitalWrite(RELAY_PIN, HIGH);   // OFF
    digitalWrite(BUZZER_PIN, HIGH);

    display.println("FAULT DETECTED");

    if (temperature > tempThreshold) {
      display.println("HIGH TEMP");
    } else {
      display.println("VIBRATION");
    }

    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");
  }

  else {
    digitalWrite(RELAY_PIN, LOW);   // ON
    digitalWrite(BUZZER_PIN, LOW);

    display.println("SYSTEM NORMAL");
    display.println("Machine Running");

    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");
  }

  display.display();
  delay(1000);
}