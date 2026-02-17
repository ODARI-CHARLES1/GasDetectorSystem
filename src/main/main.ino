#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial mySerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// LCD (change 0x27 if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin Definitions
const int mq135Pin = A0;
const int redLED = 7;
const int greenLED = 6;

int gasThreshold = 400;  // Adjust after calibration
bool gasAlertActive = false;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Gas Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer not detected!");
    lcd.print("DFPlayer Error");
    while (true);
  }

  myDFPlayer.volume(25);
  Serial.println("System Ready");
}

void loop() {

  int gasValue = analogRead(mq135Pin);

  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(gasValue);
  lcd.print("    "); // Clear extra digits

  if (gasValue > gasThreshold) {

    // 🚨 GAS DETECTED
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);

    lcd.setCursor(0, 1);
    lcd.print("!!! GAS LEAK !!!");

    if (!gasAlertActive) {
      myDFPlayer.play(1);
      gasAlertActive = true;
    }

  } else {

    // ✅ SAFE
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);

    lcd.setCursor(0, 1);
    lcd.print("Status: SAFE     ");

    gasAlertActive = false;
  }

  delay(1000);
}
