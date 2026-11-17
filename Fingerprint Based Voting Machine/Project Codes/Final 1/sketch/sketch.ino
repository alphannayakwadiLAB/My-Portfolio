#define BLYNK_TEMPLATE_ID "TMPL3Jy0mHmaB"
#define BLYNK_TEMPLATE_NAME "Smart Hydration Meter"
#define BLYNK_AUTH_TOKEN "Lp7QTacGumVz4kEYg5Ljd5_KIRcx7HVd"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX711.h"

// WiFi
char ssid[] = "Nayakwadi";
char pass[] = "12345678";

// HX711 Pins
#define DT D5
#define SCK D6

HX711 scale;

// Calibration factor
float calibration_factor = -7050;

// Variables
float initial_weight = 0;
float current_weight = 0;
float water_intake = 0;

int daily_goal = 2000;

// Timer
BlynkTimer timer;

// Alert flag
bool alert_sent = false;

// ------------------ BLYNK INPUT ------------------
BLYNK_WRITE(V2) {
  daily_goal = param.asInt();
  Serial.print("New Daily Goal: ");
  Serial.println(daily_goal);
}

BLYNK_WRITE(V4) {
  if (param.asInt() == 1) {
    initial_weight = scale.get_units(10);
    water_intake = 0;
    alert_sent = false;
    Serial.println("Bottle Reset Done");
  }
}

// ------------------ FUNCTION ------------------
void readWeight() {
  current_weight = scale.get_units(5);

  if (current_weight < 0) current_weight = 0;

  water_intake = initial_weight - current_weight;
  if (water_intake < 0) water_intake = 0;

  // Serial Output
  Serial.print("Current Weight: ");
  Serial.print(current_weight);
  Serial.print(" g | Water Intake: ");
  Serial.println(water_intake);

  // Send to Blynk
  Blynk.virtualWrite(V0, current_weight);
  Blynk.virtualWrite(V1, water_intake);

  // Alert
  if (water_intake < daily_goal) {
    Serial.println("⚠️ LOW WATER - DRINK WATER");

    Blynk.virtualWrite(V3, 255);

    if (!alert_sent) {
      Blynk.logEvent("low_water", "⚠️ Drink Water!");
      alert_sent = true;
    }

  } else {
    Serial.println("✅ Hydration OK");

    Blynk.virtualWrite(V3, 0);
    alert_sent = false;
  }
}

// ------------------ DAILY RESET ------------------
void dailyReset() {
  initial_weight = scale.get_units(10);
  water_intake = 0;
  alert_sent = false;

  Serial.println("🔄 Daily Reset Done");
}

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(115200);   // Faster serial
  delay(1000);

  Serial.println("\n===== SMART HYDRATION SYSTEM =====");

  // HX711 setup
  Serial.println("Initializing HX711...");
  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();
  Serial.println("HX711 Ready");

  Serial.println("Place full bottle...");
  delay(5000);

  initial_weight = scale.get_units(10);

  Serial.print("Initial Weight: ");
  Serial.println(initial_weight);

  // -------- WIFI CONNECT --------
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // -------- BLYNK CONNECT --------
  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  if (Blynk.connected()) {
    Serial.println("✅ Blynk Connected!");
  } else {
    Serial.println("❌ Blynk Not Connected!");
  }

  // Timers
  timer.setInterval(2000L, readWeight);
  timer.setInterval(86400000L, dailyReset);
}

// ------------------ LOOP ------------------
void loop() {
  Blynk.run();
  timer.run();
}