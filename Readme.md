# IoT Gas Detection System

A simple and effective gas leak detection system built with Arduino Uno and MQ-2 gas sensor. This project detects dangerous gas levels and provides visual and audio alerts when gas is detected.

## Table of Contents

- [Overview](#overview)
- [Hardware Components](#hardware-components)
- [Pin Connections](#pin-connections)
- [How It Works](#how-it-works)
- [Installation](#installation)
- [Wokwi Simulation](#wokwi-simulation)
- [Code Explanation](#code-explanation)
- [Calibration](#calibration)
- [Safety Notes](#safety-notes)

## Overview

This IoT Gas Detection System uses the MQ-2 gas sensor to detect the presence of dangerous gases like:
- LPG (Liquefied Petroleum Gas)
- Propane
- Methane
- Hydrogen
- Smoke

When gas concentration exceeds a predefined threshold, the system triggers:
- Red LED to turn ON
- Green LED to turn OFF  
- Buzzer to sound an alarm

## Hardware Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| Arduino Uno | 1 | Main controller |
| MQ-2 Gas Sensor | 1 | Gas detection |
| Red LED | 1 | Danger indicator |
| Green LED | 1 | Safe indicator |
| Buzzer | 1 | Audio alarm |
| Resistors (220Ω) | 2 | LED current limiting |
| Jumper Wires | Several | Connections |

## Pin Connections

| Arduino Pin | Component | Description |
|-------------|-----------|-------------|
| A0 | MQ-2 AOUT | Analog gas value |
| D5 | MQ-2 DOUT | Digital gas detection |
| D2 | Red LED | Danger indicator |
| D3 | Green LED | Safe indicator |
| D4 | Buzzer | Audio alarm |
| 3.3V | MQ-2 VCC | Power (3.3V) |
| GND | MQ-2 GND | Ground |

### MQ-2 Pinout
- **VCC**: Connect to 3.3V or 5V
- **GND**: Connect to GND
- **DOUT**: Digital output (0 or 1 based on threshold)
- **AOUT**: Analog output (continuous gas concentration)

## How It Works

1. **Initialization**: On startup, the system initializes all pins and starts with the Green LED ON (safe condition)

2. **Continuous Monitoring**: The Arduino continuously reads:
   - Digital value from MQ-2 sensor (threshold-based)
   - Analog value from MQ-2 sensor (concentration level)

3. **Detection Logic**:
   - If analog value ≥ threshold (default: 400):
     - Red LED turns ON
     - Green LED turns OFF
     - Buzzer activates
     - "WARNING: Gas leak detected!" printed to serial
   - If analog value < threshold:
     - Red LED turns OFF
     - Green LED turns ON
     - Buzzer stays silent
     - "Environment Safe" printed to serial

4. **Loop**: The check repeats every 1 second

## Installation

### Hardware Setup
1. Connect all components according to the Pin Connections table
2. Ensure proper power supply (USB or 9V barrel jack)

### Software Setup
1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Open `src/main.ino` in Arduino IDE
3. Select your board: **Tools → Board → Arduino Uno**
4. Select correct port: **Tools → Port**
5. Upload the code: **Sketch → Upload**

### Serial Monitor
1. After uploading, open Serial Monitor: **Tools → Serial Monitor**
2. Set baud rate to **9600**
3. View real-time gas readings and status

## Wokwi Simulation

This project can be simulated online using [Wokwi](https://wokwi.com/).

### Running the Simulation
1. Open [Wokwi Arduino Simulator](https://wokwi.com/arduino)
2. Create a new project or import `src/Diagram.json`
3. The simulation includes:
   - Arduino Uno
   - MQ-2 Gas Sensor with adjustable threshold

### Diagram Configuration
The simulation configuration is saved in `src/Diagram.json`.

## Code Explanation

```cpp
// Pin definitions
const int mqDigitalPin = 5;   // MQ-2 digital output
const int mqAnalogPin = A0;   // MQ-2 analog output
const int redLedPin = 2;
const int greenLedPin = 3;
const int buzzerPin = 4;

// Gas threshold (adjust based on calibration)
const int gasThreshold = 400; // Analog reading threshold (0-1023)
```

### Key Functions

- `setup()`: Initializes all pins as INPUT or OUTPUT and sets initial LED states
- `loop()`: Main detection loop that runs continuously
- `analogRead()`: Reads analog gas concentration (0-1023)
- `digitalRead()`: Reads digital threshold output from sensor
- `digitalWrite()`: Controls LEDs and buzzer

## Calibration

The MQ-2 sensor may require calibration for your specific environment:

1. **Adjusting Threshold**:
   ```cpp
   const int gasThreshold = 400; // Increase for less sensitivity, decrease for more
   ```

2. **Preheating**: Allow MQ-2 to warm up for 24-48 hours for accurate readings

3. **Testing**: 
   - Place sensor in fresh air (should show low values)
   - Expose to gas source (should show high values)
   - Adjust threshold accordingly

## Safety Notes

⚠️ **Important Safety Information**:

- This system is for **educational and demonstration purposes** only
- Do NOT use for life-safety applications without proper certification
- MQ-2 sensors have limited accuracy and can degrade over time
- Always have professional gas detection systems for real-world safety
- The sensor requires proper ventilation and mounting
- Handle the sensor carefully - it's fragile

## Project Structure

```
GasDetectorSystem/
├── .gitignore
├── Readme.md                 # This file
├── Wokwi.toml               # Wokwi simulation config
├── Docs/
│   └── GasDetector.docx     # Documentation
├── Schematic/               # Circuit diagrams (placeholder)
└── src/
    ├── main.ino            # Main Arduino code
    └── Diagram.json        # Wokwi diagram config
```

## Serial Output Example

```
MQ-2 Digital: 0 | MQ-2 Analog: 150
Environment Safe
MQ-2 Digital: 0 | MQ-2 Analog: 180
Environment Safe
MQ-2 Digital: 1 | MQ-2 Analog: 450
WARNING: Gas leak detected!
MQ-2 Digital: 1 | MQ-2 Analog: 520
WARNING: Gas leak detected!
```

## Future Improvements

Potential enhancements for this project:
- Add OLED display for local status
- Integrate WiFi module (ESP8266/ESP32) for IoT alerts
- Add SMS notifications
- Add multiple gas sensors
- Add battery backup
- Add calibration button
- Add data logging

## License

This project is provided as-is for educational purposes.

## Author

Created for IoT gas detection demonstration purposes.

---

*Last updated: February 2026*
