// ----- Blynk Template Info -----
#define BLYNK_TEMPLATE_ID "TMPL3NFPutRMz"     // Replace with your actual Template ID
#define BLYNK_TEMPLATE_NAME "Relay Control"
#define BLYNK_AUTH_TOKEN "SLSrhA9p9moeIP773Ta76qLZBwpFIGJ_" // Replace with your Blynk Auth Token

// ----- Libraries -----
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ----- WiFi Credentials -----
char auth[] = "SLSrhA9p9moeIP773Ta76qLZBwpFIGJ_";
char ssid[] = "JioPrivateNet";      // 🔹 Enter your WiFi name
char pass[] = "1020304050";         // 🔹 Enter your WiFi password

// ----- Relay Pin -----
#define RELAY_PIN 5   // GPIO5 (Connect relay signal pin here)

// ----- BLYNK Virtual Pin -----
BLYNK_WRITE(V0)  // Widget Button on Blynk app linked to V0
{
  int pinValue = param.asInt();  // 1 = ON, 0 = OFF
  digitalWrite(RELAY_PIN, pinValue);
  Serial.print("Relay State: ");
  Serial.println(pinValue);
}

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Relay OFF initially

  // Connect to Wi-Fi and Blynk Cloud
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  Serial.println("Connected to Blynk Cloud!");
}

void loop()
{
  Blynk.run();
}
