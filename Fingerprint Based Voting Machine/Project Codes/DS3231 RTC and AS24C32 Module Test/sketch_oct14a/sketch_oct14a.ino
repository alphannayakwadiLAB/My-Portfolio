#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial fingerSerial(D6, D5); // RX, TX

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup()
{
  Serial.begin(9600);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint Sensor Found!");
  } else {
    Serial.println("Fingerprint Sensor Not Found");
    while (1);
  }
}

void loop()
{
  getFingerprintID();
  delay(50);
}

uint8_t getFingerprintID() {

  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK) {
    return p;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    return p;
  }

  p = finger.fingerSearch();

  if (p == FINGERPRINT_OK) {
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence ");
    Serial.println(finger.confidence);
  }
  else {
    Serial.println("Fingerprint not found");
  }

  return finger.fingerID;
}