#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

// D5 = RX, D6 = TX
SoftwareSerial mySerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int id;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nFingerprint Enrollment System");

  mySerial.begin(57600);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Sensor detected ✅");
  } else {
    Serial.println("Sensor NOT detected ❌");
    while (1);
  }

  Serial.println("\nEnter ID (1-127): ");
  while (!Serial.available());
  id = Serial.parseInt();

  Serial.print("Enrolling ID: ");
  Serial.println(id);

  enrollFingerprint(id);
}

void loop() {
  // Nothing here (Enroll only once)
}

// 🔐 ENROLL FUNCTION
void enrollFingerprint(int id) {
  int p = -1;

  Serial.println("Place finger...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image ❌");
    return;
  }

  Serial.println("Remove finger...");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image ❌");
    return;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints did not match ❌");
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully ✅");
    Serial.print("Saved ID: ");
    Serial.println(id);
  } else {
    Serial.println("Error storing fingerprint ❌");
  }
}