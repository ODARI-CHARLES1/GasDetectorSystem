// IoT Gas Detection System for Arduino Uno
// MQ-2 Gas Sensor, Buzzer, Red/Green LEDs

// Pin definitions
const int mqDigitalPin = 5;   // MQ-2 digital output
const int mqAnalogPin = A0;   // MQ-2 analog output (optional for calibration)
const int redLedPin = 2;
const int greenLedPin = 3;
const int buzzerPin = 4;

// Gas threshold (adjust based on MQ-2 calibration)
const int gasThreshold = 400; // Analog reading threshold (0-1023)

// Variables
int gasValue = 0;

void setup() {
  Serial.begin(9600);        // Initialize serial monitor
  pinMode(mqDigitalPin, INPUT);
  pinMode(mqAnalogPin, INPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);  // Start with green LED ON
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  // Read digital and analog values from MQ-2
  int digitalGas = digitalRead(mqDigitalPin);
  gasValue = analogRead(mqAnalogPin);

  Serial.print("MQ-2 Digital: ");
  Serial.print(digitalGas);
  Serial.print(" | MQ-2 Analog: ");
  Serial.println(gasValue);

  // Check if gas exceeds threshold
  if (gasValue >= gasThreshold) {
    // Gas detected
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(buzzerPin, HIGH);
    Serial.println("WARNING: Gas leak detected!");
  } else {
    // Safe condition
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(buzzerPin, LOW);
    Serial.println("Environment Safe");
  }

  delay(1000); // Wait 1 second before next reading
}
