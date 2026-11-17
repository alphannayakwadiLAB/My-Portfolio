#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("R307 Fingerprint Sensor Test");

  finger.begin(57600);

  if (finger.verifyPassword()) 
  {
    Serial.println("Fingerprint sensor detected!");
  } 
  else 
  {
    Serial.println("Fingerprint sensor not found");
    while (1);
  }
}

void loop()
{
  Serial.println("Place finger on sensor...");
  
  int p = finger.getImage();
  
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Finger detected!");
  }
  else
  {
    Serial.println("Waiting for finger...");
  }

  delay(1000);
}