#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <DS1302.h>

// ---------------- LCD ----------------
const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);

// ---------------- MQ2 ----------------
const int mq2Pin = A0;
int sensorValue = 0;

// Thresholds
const int SMOKE_THRESHOLD = 400;
const int GAS_THRESHOLD = 500;

// ---------------- STATUS LEDS ----------------
const int greenLED = A4;
const int redLED = A5;

// ---------------- RGB LED ----------------
const int redRGB = A1;
const int greenRGB = A2;
const int blueRGB = A3;

// ---------------- DFPLAYER ----------------
SoftwareSerial dfSerial(10,11);
DFRobotDFPlayerMini player;

// ---------------- SIM800L ----------------
SoftwareSerial sim800(3,2);

// ---------------- RTC ----------------
const int RST = 12;
const int DAT = 13;
const int CLK = A6;
DS1302 rtc(RST,DAT,CLK);

// Status flags
bool gasSmsSent = false;
bool smokeSmsSent = false;
bool alarmPlaying = false;

void setup()
{

Serial.begin(9600);
dfSerial.begin(9600);
sim800.begin(9600);

lcd.begin(16,2);

// LED setup
pinMode(greenLED,OUTPUT);
pinMode(redLED,OUTPUT);

pinMode(redRGB,OUTPUT);
pinMode(greenRGB,OUTPUT);
pinMode(blueRGB,OUTPUT);

// -------- DFPLAYER --------
if(!player.begin(dfSerial))
{
Serial.println("DFPlayer Error");
}
else
{
player.volume(30);
}

// -------- RTC --------
rtc.halt(false);
rtc.writeProtect(false);

// Set RTC time ONCE then comment
// rtc.time(Time(2026,3,13,12,00,00));

// -------- SIM800 SETUP --------
delay(3000);
sim800.println("AT");
delay(2000);
sim800.println("AT+CMGF=1");
delay(2000);

// -------- SENSOR WARMUP --------
lcd.print("Sensor Warming");
lcd.setCursor(0,1);
lcd.print("Please Wait...");
delay(20000);

lcd.clear();
lcd.print("SMART GAS SYSTEM");
delay(2000);
lcd.clear();

}

void loop()
{

Time t = rtc.time();

sensorValue = analogRead(mq2Pin);

// Display info
displayLCD(t,sensorValue);

// -------- DETECTION --------

if(sensorValue > GAS_THRESHOLD)
{
handleGasDetected();
}
else if(sensorValue >= SMOKE_THRESHOLD && sensorValue <= GAS_THRESHOLD)
{
handleSmokeDetected();
}
else
{
handleAirClean();
}

delay(1000);

}

// ---------------- LCD DISPLAY ----------------

void displayLCD(Time t, int value)
{

lcd.setCursor(0,0);

String hr = (t.hr<10?"0":"") + String(t.hr);
String mn = (t.min<10?"0":"") + String(t.min);
String sc = (t.sec<10?"0":"") + String(t.sec);

String timeStr = hr + ":" + mn + ":" + sc;

String gasStr = String(value);

while(gasStr.length()<4)
gasStr = " " + gasStr;

lcd.print(timeStr + " " + gasStr + " ");

}

// ---------------- GAS DETECTED ----------------

void handleGasDetected()
{

lcd.setCursor(0,1);
lcd.print("!!! GAS ALERT !!!");

digitalWrite(redLED,HIGH);
digitalWrite(greenLED,LOW);

rgbSequence();

// Play alarm sound
if(!alarmPlaying)
{
player.play(1);
alarmPlaying = true;
}

// Send SMS once
if(!gasSmsSent)
{
sendGasSMS();
gasSmsSent = true;
}

}

// ---------------- SMOKE DETECTED ----------------

void handleSmokeDetected()
{

lcd.setCursor(0,1);
lcd.print("Smoke Detected  ");

digitalWrite(redLED,LOW);
digitalWrite(greenLED,HIGH);

// Yellow RGB
digitalWrite(redRGB,HIGH);
digitalWrite(greenRGB,HIGH);
digitalWrite(blueRGB,LOW);

// Play warning sound
if(!alarmPlaying)
{
player.play(2);
alarmPlaying = true;
}

// Send SMS once
if(!smokeSmsSent)
{
sendSmokeSMS();
smokeSmsSent = true;
}

}

// ---------------- AIR CLEAN ----------------

void handleAirClean()
{

lcd.setCursor(0,1);
lcd.print("Air Clean       ");

digitalWrite(greenLED,HIGH);
digitalWrite(redLED,LOW);

// Turn off RGB
digitalWrite(redRGB,LOW);
digitalWrite(greenRGB,LOW);
digitalWrite(blueRGB,LOW);

// Reset flags
gasSmsSent = false;
smokeSmsSent = false;
alarmPlaying = false;

}

// ---------------- RGB WARNING ----------------

void rgbSequence()
{

int colors[6][3] =
{
{1,0,0},
{0,1,0},
{0,0,1},
{1,1,0},
{1,0,1},
{0,1,1}
};

for(int i=0;i<6;i++)
{

digitalWrite(redRGB,colors[i][0]);
digitalWrite(greenRGB,colors[i][1]);
digitalWrite(blueRGB,colors[i][2]);

delay(200);

}

}

// ---------------- GAS SMS ----------------

void sendGasSMS()
{

sim800.println("AT+CMGF=1");
delay(2000);

sim800.println("AT+CMGS=\"+2547XXXXXXXX\"");  
delay(2000);

sim800.print("WARNING! GAS LEAK DETECTED!");
delay(1000);

sim800.write(26);

delay(5000);

}

// ---------------- SMOKE SMS ----------------

void sendSmokeSMS()
{

sim800.println("AT+CMGF=1");
delay(2000);

sim800.println("AT+CMGS=\"+2547XXXXXXXX\"");  
delay(2000);

sim800.print("WARNING! SMOKE DETECTED!");
delay(1000);

sim800.write(26);

delay(5000);

}