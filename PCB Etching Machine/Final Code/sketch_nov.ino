#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#define MOTOR_IN1 2
#define MOTOR_IN2 3
#define LIMIT1 4
#define LIMIT2 5
#define TIMER5_KEY 6
#define TIMER10_KEY A0
#define EMERGENCY_KEY A1
#define BUZZER 13

unsigned long timerDuration = 0;
unsigned long startTime = 0;
bool timerRunning = false;
int motorDirection = 1; // 1=CW, 2=CCW

void setup() {
  lcd.begin(16,2);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(LIMIT1, INPUT_PULLUP);
  pinMode(LIMIT2, INPUT_PULLUP);
  pinMode(TIMER5_KEY, INPUT_PULLUP);
  pinMode(TIMER10_KEY, INPUT_PULLUP);
  pinMode(EMERGENCY_KEY, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(BUZZER, LOW);

  // --- Welcome Screens ---
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("PCB ETCHING");
  lcd.setCursor(0,1); lcd.print("MACHINE");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("PROJECT BY");
  lcd.setCursor(0,1); lcd.print("ALPHAN");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("PRESS TIMER KEY");
  lcd.setCursor(0,1); lcd.print("5MIN OR 10MIN");
}

void loop() {
  // Emergency Stop
  if(digitalRead(EMERGENCY_KEY)==LOW){
    stopMotor();
    timerRunning=false;
    lcd.clear(); lcd.setCursor(0,0); lcd.print(" EMERGENCY STOP ");
    delay(2000);
    lcd.clear(); lcd.setCursor(0,0); lcd.print("PRESS TIMER KEY");
    lcd.setCursor(0,1); lcd.print("5MIN OR 10MIN");
  }

  // Start Timer
  if(!timerRunning){
    if(digitalRead(TIMER5_KEY)==LOW){
      timerDuration = 5UL*60UL*1000UL; //5min
      startTimer();
    }
    else if(digitalRead(TIMER10_KEY)==LOW){
      timerDuration = 10UL*60UL*1000UL; //10min
      startTimer();
    }
  }

  // Timer Running
  if(timerRunning){
    unsigned long elapsed = millis() - startTime;
    if(elapsed >= timerDuration){
      stopMotor();
      buzzerAlert();
      timerRunning=false;
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("TIMER COMPLETE");
      lcd.setCursor(0,1); lcd.print("PRESS TIMER KEY");
    } else {
      handleMotor();
      showRemainingTime(timerDuration - elapsed);
    }
  }
}

// ---- FUNCTIONS ----
void startTimer(){
  startTime = millis();
  timerRunning = true;
  motorDirection = 1; // start CW
  lcd.clear(); lcd.setCursor(0,0); lcd.print("ETCHING STARTED");
  lcd.setCursor(0,1); lcd.print("RUNING MOTOR");
  motorClockwise();
}

void handleMotor(){
  if(digitalRead(LIMIT1)==LOW) motorDirection = 2; // CCW
  else if(digitalRead(LIMIT2)==LOW) motorDirection = 1; // CW

  if(motorDirection==1) motorClockwise();
  else if(motorDirection==2) motorAnticlockwise();
}

void motorClockwise(){
  digitalWrite(MOTOR_IN1,HIGH);
  digitalWrite(MOTOR_IN2,LOW);
}

void motorAnticlockwise(){
  digitalWrite(MOTOR_IN1,LOW);
  digitalWrite(MOTOR_IN2,HIGH);
}

void stopMotor(){
  digitalWrite(MOTOR_IN1,LOW);
  digitalWrite(MOTOR_IN2,LOW);
}

void buzzerAlert(){
  for(int i=0;i<10;i++){
    digitalWrite(BUZZER,HIGH); delay(500);
    digitalWrite(BUZZER,LOW); delay(500);
  }
}

void showRemainingTime(unsigned long remaining){
  lcd.setCursor(0,1);
  lcd.print("TIME LEFT: ");
  int seconds = remaining/1000;
  int minutes = seconds/60;
  seconds = seconds%60;
  if(minutes<10) lcd.print('0');
  lcd.print(minutes);
  lcd.print(':');
  if(seconds<10) lcd.print('0');
  lcd.print(seconds);
  lcd.print("   "); // clear extra chars
}
