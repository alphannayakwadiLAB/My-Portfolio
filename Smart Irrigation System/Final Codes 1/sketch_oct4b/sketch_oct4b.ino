#define BLYNK_TEMPLATE_ID "TMPL3NFPutRMz"
#define BLYNK_TEMPLATE_NAME "Project"
#define BLYNK_AUTH_TOKEN "xtJcul0659vSPILQO7b0lw37_UITfRtV"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "vivo Y53s";
char pass[] = "00000000";

// ---------------- Pin Definitions ----------------
#define MOISTURE_PIN   34   // Analog input
#define TRIG_PIN       5
#define ECHO_PIN       18
#define RELAY_PIN      25
#define SWITCH_ON_PIN  33
#define SWITCH_OFF_PIN 32
#define GREEN_LED_PIN  26
#define RED_LED_PIN    27

// ---------------- Blynk Virtual Pins ----------------
// V0 = Moisture
// V1 = Ultrasonic Water Level
// V2 = Pump Switch
// V3 = Green LED status
// V4 = Red LED status
// V5 = Crop Type

BlynkTimer timer;

// Pump status
bool pumpState = false;  // false = OFF, true = ON

// Ultrasonic sensor mounting height from land
const float sensorHeight = 40.0; // cm

// Selected crop and required water level
int cropType = 1;
float targetWaterLevel = 5.0; // default (Wheat)

// --------------------------------------------------
// Function: Set crop water level based on crop type
// --------------------------------------------------
void setCropLevel(int crop)
{
  cropType = crop;

  switch (cropType)
  {
    case 1: // Wheat
      targetWaterLevel = 5.0;
      Serial.println("Crop: Wheat | Target Water Level = 5 cm");
      break;

    case 2: // Rice
      targetWaterLevel = 12.0;
      Serial.println("Crop: Rice | Target Water Level = 12 cm");
      break;

    case 3: // Sugarcane
      targetWaterLevel = 8.0;
      Serial.println("Crop: Sugarcane | Target Water Level = 8 cm");
      break;

    case 4: // Vegetables
      targetWaterLevel = 4.0;
      Serial.println("Crop: Vegetables | Target Water Level = 4 cm");
      break;

    default:
      targetWaterLevel = 5.0;
      Serial.println("Default Crop | Target Water Level = 5 cm");
      break;
  }
}

// --------------------------------------------------
// Function: Read soil moisture sensor
// --------------------------------------------------
int readMoisture()
{
  int rawValue = analogRead(MOISTURE_PIN);  // 0 to 4095
  return rawValue;
}

// Optional: Convert moisture raw value to percentage
int moisturePercentage(int rawValue)
{
  // Adjust these values after calibration
  int dryValue = 3500;   // sensor in dry soil
  int wetValue = 1200;   // sensor in wet soil

  int percent = map(rawValue, dryValue, wetValue, 0, 100);

  if (percent > 100) percent = 100;
  if (percent < 0) percent = 0;

  return percent;
}

// --------------------------------------------------
// Function: Read ultrasonic distance in cm
// --------------------------------------------------
float readDistanceCM()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms

  if (duration == 0)
  {
    return -1; // no reading
  }

  float distance = duration * 0.0343 / 2.0;
  return distance;
}

// --------------------------------------------------
// Function: Calculate water level from land surface
// Water Level = Sensor Height - Distance
// --------------------------------------------------
float getWaterLevel()
{
  float distance = readDistanceCM();

  if (distance < 0)
    return -1;

  float waterLevel = sensorHeight - distance;

  if (waterLevel < 0) waterLevel = 0;
  if (waterLevel > sensorHeight) waterLevel = sensorHeight;

  return waterLevel;
}

// --------------------------------------------------
// Function: Update pump output and LED indicators
// Relay assumed ACTIVE LOW
// --------------------------------------------------
void updatePumpOutput()
{
  if (pumpState)
  {
    digitalWrite(RELAY_PIN, LOW);   // Pump ON (active LOW relay)
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);

    Blynk.virtualWrite(V3, 1);
    Blynk.virtualWrite(V4, 0);
  }
  else
  {
    digitalWrite(RELAY_PIN, HIGH);  // Pump OFF
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);

    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V4, 1);
  }

  // Sync pump switch status back to app
  Blynk.virtualWrite(V2, pumpState ? 1 : 0);
}

// --------------------------------------------------
// Function: Send sensor data to Blynk and auto logic
// --------------------------------------------------
void sendSensorData()
{
  int moistureRaw = readMoisture();
  int moisturePercent = moisturePercentage(moistureRaw);

  float waterLevel = getWaterLevel();

  // Send to Blynk
  Blynk.virtualWrite(V0, moisturePercent);  // Moisture %
  if (waterLevel >= 0)
  {
    Blynk.virtualWrite(V1, waterLevel);     // Water level in cm
  }

  // Serial Monitor
  Serial.print("Moisture Raw: ");
  Serial.print(moistureRaw);
  Serial.print(" | Moisture %: ");
  Serial.print(moisturePercent);
  Serial.print(" | Water Level: ");
  Serial.print(waterLevel);
  Serial.print(" cm | Target: ");
  Serial.print(targetWaterLevel);
  Serial.println(" cm");

  // Auto OFF Logic:
  // If pump is ON and water reaches target level, turn OFF pump
  if (pumpState && waterLevel >= targetWaterLevel && waterLevel >= 0)
  {
    pumpState = false;
    updatePumpOutput();

    Serial.println("Target water level reached. Pump turned OFF automatically.");

    // Optional notification/log event
    Blynk.logEvent("pump_status", "Target water level reached. Pump OFF automatically.");
  }
}

// --------------------------------------------------
// Function: Check manual switches
// --------------------------------------------------
void checkManualSwitches()
{
  // Buttons with INPUT_PULLUP => pressed = LOW
  if (digitalRead(SWITCH_ON_PIN) == LOW)
  {
    pumpState = true;
    updatePumpOutput();
    Serial.println("Pump ON by manual switch");
    delay(250); // debounce
  }

  if (digitalRead(SWITCH_OFF_PIN) == LOW)
  {
    pumpState = false;
    updatePumpOutput();
    Serial.println("Pump OFF by manual switch");
    delay(250); // debounce
  }
}

// --------------------------------------------------
// Blynk: Pump control from app (V2)
// --------------------------------------------------
BLYNK_WRITE(V2)
{
  int value = param.asInt();

  if (value == 1)
  {
    pumpState = true;
    Serial.println("Pump ON from Blynk app");
  }
  else
  {
    pumpState = false;
    Serial.println("Pump OFF from Blynk app");
  }

  updatePumpOutput();
}

// --------------------------------------------------
// Blynk: Crop selection from app (V5)
// Example menu values:
// 1 = Wheat
// 2 = Rice
// 3 = Sugarcane
// 4 = Vegetables
// --------------------------------------------------
BLYNK_WRITE(V5)
{
  int selectedCrop = param.asInt();
  setCropLevel(selectedCrop);

  Serial.print("Crop selected from Blynk: ");
  Serial.println(selectedCrop);
}

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup()
{
  Serial.begin(115200);

  // Pin modes
  pinMode(MOISTURE_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);

  pinMode(SWITCH_ON_PIN, INPUT_PULLUP);
  pinMode(SWITCH_OFF_PIN, INPUT_PULLUP);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  // Initial states
  pumpState = false;
  updatePumpOutput();

  // Default crop
  setCropLevel(1);

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timers
  timer.setInterval(1000L, sendSensorData);      // every 1 second
  timer.setInterval(200L, checkManualSwitches);  // every 200 ms
}

// --------------------------------------------------
// Loop
// --------------------------------------------------
void loop()
{
  Blynk.run();
  timer.run();
}