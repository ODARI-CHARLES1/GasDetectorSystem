#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ================= PIN DEFINITIONS =================
LiquidCrystal lcd(4,5,6,7,8,9);

const int mq2Pin = A0;      // Smoke/Gas sensor
const int flamePin = A1;    // Flame sensor

const int redRGB = A2;
const int greenRGB = 12;
const int blueRGB = A4;

const int redLED = 13;

// ================= DFPLAYER =================
SoftwareSerial dfSerial(10,11);
DFRobotDFPlayerMini player;

// ================= SIM800L =================
SoftwareSerial sim800(3,2); // RX, TX pins
const char phoneNumber[] = "+254797933796"; // Replace with your number

// ================= THRESHOLDS =================
const int SMOKE_THRESHOLD = 380;
const int GAS_THRESHOLD   = 420;
const int FIRE_THRESHOLD  = 600;

// ================= RGB BLINK =================
unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 500;
bool blinkState = false;

void setRGB(int r,int g,int b){
  digitalWrite(redRGB,r);
  digitalWrite(greenRGB,g);
  digitalWrite(blueRGB,b);
}

void blinkRGB(int r,int g,int b){
  if(millis() - lastBlink >= BLINK_INTERVAL){
    lastBlink = millis();
    blinkState = !blinkState;
    if(blinkState)
      setRGB(r,g,b);
    else
      setRGB(0,0,0);
  }
}

void setup(){
  Serial.begin(9600);
  sim800.begin(9600);
  dfSerial.begin(9600);
  lcd.begin(16,2);

  pinMode(redRGB,OUTPUT);
  pinMode(greenRGB,OUTPUT);
  pinMode(blueRGB,OUTPUT);
  pinMode(redLED,OUTPUT);
  pinMode(flamePin,INPUT);

  setRGB(0,1,0); // GREEN = SAFE

  lcd.print("Smart Safety Sys");
  lcd.setCursor(0,1);
  lcd.print("Initializing..");
  delay(1000);

  player.begin(dfSerial);
  player.volume(30);

  // Test SIM800L
  sim800.println("AT");
  delay(500);

  lcd.clear();
  lcd.print("Sensor Warmup");
  lcd.setCursor(0,1);
  lcd.print("Please wait");
  delay(3000);
  lcd.clear();
}

void loop(){
  int gasValue = analogRead(mq2Pin);
  int flameValue = analogRead(flamePin);

  Serial.print("Gas:"); Serial.print(gasValue);
  Serial.print(" Flame:"); Serial.println(flameValue);

  int currentState = 0;
  if(flameValue >= FIRE_THRESHOLD) currentState = 3;
  else if(gasValue >= GAS_THRESHOLD) currentState = 2;
  else if(gasValue >= SMOKE_THRESHOLD) currentState = 1;
  else currentState = 0;

  // ===== SYSTEM ACTIONS =====
  switch(currentState){
    case 0: // SAFE
      digitalWrite(redLED,LOW);
      setRGB(0,1,0);
      lcd.home();
      lcd.print("GAS LEVEL:");
      lcd.print(gasValue);
      lcd.setCursor(0,1);
      lcd.print("SYSTEM SAFE    ");
      break;

    case 1: // SMOKE
    case 2: // GAS
    case 3: // FIRE
      digitalWrite(redLED,HIGH);
      if(currentState==1) blinkRGB(1,1,0);
      if(currentState==2) blinkRGB(1,0,1);
      if(currentState==3) blinkRGB(1,0,0);

      // Play alarm once
      player.play(1);

      // Prepare SMS message
      String msg;
      if(currentState == 1) msg = "ALERT: Hazard detected! Please check immediately.!";
      if(currentState == 2) msg = "ALERT: Hazard detected! Please check immediately.!";
      if(currentState == 3) msg = "ALERT: Hazard detected! Please check immediately.!";

      // ===== SEND SMS =====
      lcd.setCursor(0,1);
      lcd.print("Sending SMS...  ");
      sim800.println("AT+CMGF=1");
      delay(500);
      sim800.print("AT+CMGS=\"");
      sim800.print(phoneNumber);
      sim800.println("\"");
      delay(500);
      sim800.print(msg);
      delay(500);
      sim800.write(26); // CTRL+Z to send
      lcd.setCursor(0,1);
      lcd.print("SMS SENT        ");
      Serial.println("SMS Sent: " + msg);

      // ===== WAIT 4 SECONDS THEN CALL =====
      delay(4000);
      lcd.setCursor(0,1);
      lcd.print("Calling...      ");
      sim800.print("ATD");
      sim800.print(phoneNumber);
      sim800.println(";");
      Serial.println("Calling " + String(phoneNumber));

      // Wait a bit so alarm/call has time before next loop
      delay(10000); // 10 seconds before reading sensors again
      break;
  }

  delay(500); // short delay for stable readings
}