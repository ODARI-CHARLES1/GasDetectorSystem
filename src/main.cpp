#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ================= PIN DEFINITIONS =================
LiquidCrystal lcd(4,5,6,7,8,9);   // RS, EN, D4-D7
const int mq2Pin = A0;
const int flamePin = A1;
const int redRGB = A2;
const int greenRGB = 12;
const int blueRGB = A4;
const int redLED = 13;

// SIM800L
SoftwareSerial sim800(3,2);

// DFPlayer
SoftwareSerial dfSerial(10,11);
DFRobotDFPlayerMini player;

// ---------------- Thresholds ----------------
const int GAS_THRESHOLD = 500;
const int SMOKE_THRESHOLD = 350;
const int FIRE_THRESHOLD = 700;

// ================= STATE =================
int gasValue = 0;
int flameValue = 0;
int currentState = 0;  // 0=SAFE, 1=SMOKE, 2=GAS, 3=FIRE
int lastState = 0;

bool smsSent = false;
bool callMade = false;
bool alarmPlaying = false;

// ================= TIMING =================
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastSmsTime = 0;
unsigned long lastCallTime = 0;
unsigned long gasFallTime = 0;

const unsigned long SENSOR_INTERVAL = 50;
const unsigned long LCD_INTERVAL = 500;
const unsigned long SMS_COOLDOWN = 30000;
const unsigned long CALL_COOLDOWN = 60000;
const unsigned long GAS_FALL_DELAY = 10000;  // 10 sec delay to allow gas to fall

// ================= RGB CONTROL =================
void setRGB(int r,int g,int b){
  digitalWrite(redRGB,r);
  digitalWrite(greenRGB,g);
  digitalWrite(blueRGB,b);
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