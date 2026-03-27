#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---------------- LCD ----------------
LiquidCrystal lcd(4,5,6,7,8,9);

// ---------------- RTC ----------------
RTC_PCF8563 rtc;

// ---------------- Sensors ----------------
const int mq2Pin = A0;
const int flamePin = A1;  // IR phototransistor analog input

// ---------------- RGB LED ----------------
const int redRGB = A2;
const int greenRGB = A3;
const int blueRGB = A4;

// ---------------- Status LEDs ----------------
const int greenLED = 12;
const int redLED = 13;

// ---------------- SIM800L ----------------
SoftwareSerial sim800(2,3);
String phoneNumber = "+254111639505";

// ---------------- DFPLAYER ----------------
SoftwareSerial dfSerial(10,11);
DFRobotDFPlayerMini player;

// ---------------- Thresholds ----------------
const int GAS_THRESHOLD = 500;
const int SMOKE_THRESHOLD = 350;
const int FIRE_THRESHOLD = 700;

// ---------------- Variables ----------------
int gasValue;
int flameValue;
bool alertActive = false;
bool gasSmsSent = false;
bool smokeSmsSent = false;
bool fireSmsSent = false;
bool callMade = false;
bool alarmPlaying = false;

// ---------------- RGB Timing ----------------
unsigned long rgbTimer = 0;
int rgbState = 0;

// ---------------- FUNCTIONS ----------------

// RGB blinking sequence for danger
void rgbDangerBlink(){
  if(millis() - rgbTimer > 200){
    rgbTimer = millis();
    switch(rgbState){
      case 0: digitalWrite(redRGB,HIGH); digitalWrite(greenRGB,LOW); digitalWrite(blueRGB,LOW); break;
      case 1: digitalWrite(redRGB,LOW); digitalWrite(greenRGB,HIGH); digitalWrite(blueRGB,LOW); break;
      case 2: digitalWrite(redRGB,LOW); digitalWrite(greenRGB,LOW); digitalWrite(blueRGB,HIGH); break;
      case 3: digitalWrite(redRGB,HIGH); digitalWrite(greenRGB,HIGH); digitalWrite(blueRGB,LOW); break;
      case 4: digitalWrite(redRGB,HIGH); digitalWrite(greenRGB,LOW); digitalWrite(blueRGB,HIGH); break;
      case 5: digitalWrite(redRGB,LOW); digitalWrite(greenRGB,HIGH); digitalWrite(blueRGB,HIGH); break;
    }
    rgbState++;
    if(rgbState > 5) rgbState = 0;
  }
}

// Send SMS via SIM800L
void sendSMS(String msg){
  sim800.println("AT+CMGF=1");
  delay(500);
  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
  delay(500);
  sim800.print(msg);
  delay(500);
  sim800.write(26); // CTRL+Z
  delay(4000);
}

// Make phone call via SIM800L
void makeCall(){
  sim800.print("ATD");
  sim800.print(phoneNumber);
  sim800.println(";");
  delay(30000); // Ring for 30 sec
  sim800.println("ATH"); // Hang up
}

// Display gas and IR sensor values on LCD
void displayLCD(DateTime now, int gas, int flame){
  lcd.setCursor(0,0);
  String hr = (now.hour()<10?"0":"") + String(now.hour());
  String mn = (now.minute()<10?"0":"") + String(now.minute());
  String sc = (now.second()<10?"0":"") + String(now.second());
  lcd.print(hr + ":" + mn + ":" + sc);

  lcd.setCursor(9,0);
  lcd.print("G:");
  lcd.print(gas);

  lcd.setCursor(0,1);
  lcd.print("IR:");
  lcd.print(flame);
}

// ---------------- SETUP ----------------
void setup(){
  Serial.begin(9600);
  Wire.begin();
  sim800.begin(9600);
  dfSerial.begin(9600);
  lcd.begin(16,2);

  pinMode(redRGB,OUTPUT);
  pinMode(greenRGB,OUTPUT);
  pinMode(blueRGB,OUTPUT);
  pinMode(redLED,OUTPUT);
  pinMode(greenLED,OUTPUT);
  pinMode(flamePin,INPUT);

  // DFPlayer
  if(player.begin(dfSerial)){
    player.volume(25);
  }

  // RTC
  if(!rtc.begin()){
    Serial.println("RTC Error");
    while(1);
  }
  if(rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  }

  lcd.print("SMART SAFETY SYS");
  delay(2000);
  lcd.clear();

  // SIM800L init
  delay(3000);
  sim800.println("AT");
  delay(1000);
  sim800.println("AT+CMGF=1");
}

// ---------------- LOOP ----------------
void loop(){
  DateTime now = rtc.now();
  gasValue = analogRead(mq2Pin);
  flameValue = analogRead(flamePin);

  displayLCD(now, gasValue, flameValue);
  Serial.print("Gas: "); Serial.print(gasValue); 
  Serial.print(" | IR: "); Serial.println(flameValue);

  alertActive = false;

  // -------- FIRE DETECTED --------
  if(flameValue > FIRE_THRESHOLD){
    alertActive = true;
    lcd.setCursor(0,1); lcd.print("FIRE DETECTED!!!");
    digitalWrite(redLED,HIGH);
    digitalWrite(greenLED,LOW);
    rgbDangerBlink();
    if(!alarmPlaying){
      player.play(3); // fire alarm sound
      alarmPlaying = true;
    }
    if(!fireSmsSent){
      sendSMS("CRITICAL ALERT!\nFire detected in monitored area!\nImmediate action required.");
      fireSmsSent = true;
    }
    if(!callMade){
      makeCall();
      callMade = true;
    }
  }
  // -------- GAS DETECTED --------
  else if(gasValue > GAS_THRESHOLD){
    alertActive = true;
    lcd.setCursor(0,1); lcd.print("GAS LEAK ALERT ");
    digitalWrite(redLED,HIGH);
    digitalWrite(greenLED,LOW);
    rgbDangerBlink();
    if(!alarmPlaying){
      player.play(1); // methane alarm
      alarmPlaying = true;
    }
    if(!gasSmsSent){
      sendSMS("EMERGENCY ALERT: Methane (CH4) gas leakage has been detected by the Smart Gas Monitoring System. Methane is highly flammable and may cause fire/explosion. Immediate inspection required.");
      gasSmsSent = true;
    }
  }
  // -------- SMOKE DETECTED --------
  else if(gasValue > SMOKE_THRESHOLD){
    alertActive = true;
    lcd.setCursor(0,1); lcd.print("SMOKE DETECTED ");
    digitalWrite(redLED,LOW);
    digitalWrite(greenLED,HIGH);
    digitalWrite(redRGB,HIGH);
    digitalWrite(greenRGB,HIGH);
    digitalWrite(blueRGB,LOW);
    if(!alarmPlaying){
      player.play(2); // smoke alarm
      alarmPlaying = true;
    }
    if(!smokeSmsSent){
      sendSMS("WARNING: Smoke has been detected by the Smart Gas and Fire Monitoring System. The presence of smoke may indicate a potential fire hazard or overheating equipment. Please inspect immediately.");
      smokeSmsSent = true;
    }
  }
  // -------- SAFE --------
  else{
    alertActive = false;
    gasSmsSent = false;
    smokeSmsSent = false;
    fireSmsSent = false;
    callMade = false;
    alarmPlaying = false;
    lcd.setCursor(0,1); lcd.print("AIR SAFE        ");
    digitalWrite(greenLED,HIGH);
    digitalWrite(redLED,LOW);
    digitalWrite(redRGB,LOW);
    digitalWrite(greenRGB,LOW);
    digitalWrite(blueRGB,LOW);
  }

  // Continue RGB blink during danger
  if(alertActive) rgbDangerBlink();

  delay(500);
}