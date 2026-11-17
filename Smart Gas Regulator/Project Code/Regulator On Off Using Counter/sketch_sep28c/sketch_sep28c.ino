// Define pins
const int mq5Pin = A0;       // MQ-5 analog output
const int buzzerPin = 8;     // Buzzer pin
const int relayFanPin = 9;   // Relay for exhaust fan
const int relayLightPin = 10;// Relay for emergency light

// Thresholds
const int gasThreshold = 100; // Percentage threshold for alarm

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayFanPin, OUTPUT);
  pinMode(relayLightPin, OUTPUT);

  // Initial state: all OFF
  digitalWrite(buzzerPin, LOW);
  digitalWrite(relayFanPin, LOW);
  digitalWrite(relayLightPin, LOW);

  Serial.begin(9600);
  Serial.println("MQ-5 Gas Sensor System Initialized...");
}

void loop() {
  int sensorValue = analogRead(mq5Pin); // Read raw analog value (0–1023)

  // Convert to percentage (approximate linear map)
  int gasPercent = map(sensorValue, 0, 1023, 0, 100);
  gasPercent = constrain(gasPercent, 0, 100); // Clamp to 0–100%

  Serial.print("Gas Level: ");
  Serial.print(gasPercent);
  Serial.println("%");

  if (gasPercent > gasThreshold) {
    // Gas leak detected (> 50%)
    digitalWrite(buzzerPin, HIGH);         // Buzzer ON
    digitalWrite(relayFanPin, HIGH);       // Exhaust ON
    digitalWrite(relayLightPin, HIGH);     // Emergency light ON

    Serial.println("⚠️  GAS LEAK DETECTED! Activating emergency systems...");
  } else {
    // Safe gas level
    digitalWrite(buzzerPin, LOW);
    digitalWrite(relayFanPin, LOW);
    digitalWrite(relayLightPin, LOW);
  }

  delay(500); // Wait before next reading
}
