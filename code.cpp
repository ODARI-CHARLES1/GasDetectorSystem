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

const char phoneNumber[] = "+254111639505"; // Replace with your number

// ================= THRESHOLDS =================
const int SMOKE_THRESHOLD = 380;
const int GAS_THRESHOLD   = 420;
const int FIRE_THRESHOLD  = 600;

// ================= STATE =================
int gasValue = 0;
int flameValue = 0;

int currentState = 0; // 0 SAFE, 1 SMOKE, 2 GAS, 3 FIRE
int lastState = 0;

bool smsSent = false;
bool alarmPlaying = false;

// ================= TIMING =================
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastSmsTime = 0;
unsigned long gasFallTime = 0;

const unsigned long SENSOR_INTERVAL = 50;
const unsigned long LCD_INTERVAL = 500;
const unsigned long SMS_COOLDOWN = 10000;   // 10 sec
const unsigned long GAS_FALL_DELAY = 5000;  // 5 sec

// ================= RGB BLINK =================
unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 500;
bool blinkState = false;

// ================= RGB CONTROL =================
void setRGB(int r,int g,int b){
  digitalWrite(redRGB,r);
  digitalWrite(greenRGB,g);
  digitalWrite(blueRGB,b);
}

// ================= RGB BLINK FUNCTION =================
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

// ================= ALERT FUNCTION =================
void sendSMS(String message){
  sim800.println("AT+CMGF=1");           // Text mode
  delay(200);
  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
  delay(200);
  sim800.print(message);
  delay(200);
  sim800.write(26);                      // CTRL+Z to send
}

// ================= SETUP =================
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
  player.volume(25);

  // Wait for SIM800L to initialize
  delay(1000);
  sim800.println("AT");       // Test command
  delay(500);

  lcd.clear();
  lcd.print("Sensor Warmup");
  lcd.setCursor(0,1);
  lcd.print("Please wait");
  delay(3000);
  lcd.clear();
}

// ================= LOOP =================
void loop(){
  unsigned long now = millis();

  // ===== SENSOR READING =====
  if(now - lastSensorRead >= SENSOR_INTERVAL){
    lastSensorRead = now;

    // Average multiple readings for gas
    int total = 0;
    for(int i=0;i<5;i++) total += analogRead(mq2Pin);
    gasValue = total/5;

    flameValue = analogRead(flamePin);

    Serial.print("Gas:"); Serial.print(gasValue);
    Serial.print(" Flame:"); Serial.println(flameValue);
  }

  // ===== STATE DETECTION =====
  if(flameValue >= FIRE_THRESHOLD) currentState = 3;
  else if(gasValue >= GAS_THRESHOLD) currentState = 2;
  else if(gasValue >= SMOKE_THRESHOLD) currentState = 1;
  else currentState = 0;

  // ===== GAS HOLD LOGIC =====
  if(lastState==2 && currentState==1){
    if(gasFallTime==0) gasFallTime = now;
    if(now - gasFallTime < GAS_FALL_DELAY) currentState = 2;
    else gasFallTime = 0;
  } else gasFallTime = 0;

  // ===== STATE CHANGE =====
  if(currentState != lastState){
    smsSent = false;
    alarmPlaying = false;
    player.stop();
    lastState = currentState;
  }

  // ===== SYSTEM ACTIONS =====
  switch(currentState){
    case 0: // SAFE
      digitalWrite(redLED,LOW);
      setRGB(0,1,0); // solid green
      break;
    case 1: // SMOKE
      digitalWrite(redLED,LOW);
      blinkRGB(1,1,0); // blinking yellow
      if(!alarmPlaying){ player.play(1); alarmPlaying = true; }
      break;
    case 2: // GAS
      digitalWrite(redLED,HIGH);
      blinkRGB(1,0,1); // blinking magenta
      if(!alarmPlaying){ player.play(1); alarmPlaying = true; }
      break;
    case 3: // FIRE
      digitalWrite(redLED,HIGH);
      blinkRGB(1,0,0); // blinking red
      if(!alarmPlaying){ player.play(1); alarmPlaying = true; }
      break;
  }

  // ===== SMS ALERT =====
  if(currentState != 0 && !smsSent && now - lastSmsTime > SMS_COOLDOWN){
    String msg;
    if(currentState == 1) msg = "WARNING: Smoke detected in the monitored area. Please check immediately to prevent potential fire hazard.";
    if(currentState == 2) msg = "EMERGENCY: Gas leak detected in the premises. Evacuate immediately and take necessary safety precautions.";
    if(currentState == 3) msg = "CRITICAL ALERT: Fire detected! Immediate action required! Evacuate and call emergency services.";

    sendSMS(msg);
    smsSent = true;
    lastSmsTime = now;
    Serial.println("SMS Sent: " + msg);
  }

  // ===== LCD UPDATE =====
  if(now - lastLCDUpdate >= LCD_INTERVAL){
    lastLCDUpdate = now;
    lcd.setCursor(0,0);
    lcd.print("Gas:"); lcd.print(gasValue); lcd.print("   ");
    lcd.setCursor(0,1);
    switch(currentState){
      case 0: lcd.print("SYSTEM SAFE    "); break;
      case 1: lcd.print("SMOKE DETECTED "); break;
      case 2: lcd.print("GAS LEAK ALERT "); break;
      case 3: lcd.print("FIRE DANGER!!  "); break;
    }
  }
}