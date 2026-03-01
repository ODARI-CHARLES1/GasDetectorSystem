#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <DS1302.h>

// -------------------- LCD SETUP --------------------
const int rs = 9, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// -------------------- MQ-2 SENSOR --------------------
const int mq2Pin = A0;
int sensorValue = 0;
const int GAS_THRESHOLD = 400;

// -------------------- LEDs --------------------
const int greenLED = 6;
const int redLED = 7;

// -------------------- RGB LED --------------------
const int redRGB = A1;
const int greenRGB = A2;
const int blueRGB = A3;

// -------------------- DFPLAYER --------------------
SoftwareSerial mySerial(10, 11); // RX, TX
DFRobotDFPlayerMini player;

// -------------------- RTC --------------------
const int RST = 1;
const int DAT = 12;
const int CLK = 13;
DS1302 rtc(RST, DAT, CLK);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  lcd.begin(16,2);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(redRGB, OUTPUT);
  pinMode(greenRGB, OUTPUT);
  pinMode(blueRGB, OUTPUT);

  if (!player.begin(mySerial)) {
    Serial.println("DFPlayer error");
  } else {
    player.volume(25);
  }

  rtc.halt(false);
  rtc.writeProtect(false);

  lcd.print("Smart Gas Monitor");
  delay(2000);
  lcd.clear();
}

// -------------------- MAIN LOOP --------------------
void loop() {
  Time t = rtc.time();
  sensorValue = analogRead(mq2Pin);

  displayLCD(t, sensorValue);

  if(sensorValue > GAS_THRESHOLD) {
    handleGasDetected();
  } else {
    handleAirClean();
  }

  delay(1000);
}

// -------------------- DISPLAY FUNCTION --------------------
void displayLCD(Time t, int value) {
  lcd.setCursor(0,0);

  // Format time HH:MM:SS
  String timeStr = formatTime(t);

  // Format gas value to fixed width
  String gasStr = String(value);
  while(gasStr.length() < 4) gasStr = " " + gasStr; // pad left

  // Combine for line 1
  lcd.print(timeStr + " " + gasStr);

  // LINE 2 handled in status functions
}

String formatTime(Time t) {
  String hr = (t.hr < 10) ? "0" + String(t.hr) : String(t.hr);
  String min = (t.min < 10) ? "0" + String(t.min) : String(t.min);
  String sec = (t.sec < 10) ? "0" + String(t.sec) : String(t.sec);
  return hr + ":" + min + ":" + sec;
}

// -------------------- STATUS FUNCTIONS --------------------
void handleGasDetected() {
  lcd.setCursor(0,1);
  lcd.print("! GAS ALERT !   "); // fixed width

  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);

  rgbSequence();
  player.play(1);
}

void handleAirClean() {
  lcd.setCursor(0,1);
  lcd.print("Air Clean      "); // fixed width

  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, LOW);

  digitalWrite(redRGB, LOW);
  digitalWrite(greenRGB, LOW);
  digitalWrite(blueRGB, LOW);
}

// -------------------- RGB SEQUENCE --------------------
void rgbSequence() {
  int colors[6][3] = {
    {1,0,0}, // RED
    {0,1,0}, // GREEN
    {0,0,1}, // BLUE
    {1,1,0}, // YELLOW
    {1,0,1}, // PURPLE
    {0,1,1}  // CYAN
  };
  for(int i=0; i<6; i++){
    digitalWrite(redRGB, colors[i][0]);
    digitalWrite(greenRGB, colors[i][1]);
    digitalWrite(blueRGB, colors[i][2]);
    delay(300);
  }
}