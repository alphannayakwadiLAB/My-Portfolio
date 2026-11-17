const int buzzerPin = 8;  // Connect buzzer + to pin 8

void setup() {
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  digitalWrite(buzzerPin, HIGH);  // Turn buzzer ON
  delay(500);                     // Wait 500ms
  digitalWrite(buzzerPin, LOW);   // Turn buzzer OFF
  delay(500);                     // Wait 500ms
}
