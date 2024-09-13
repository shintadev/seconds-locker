# Seconds-Locker

Seconds-Locker is a secure, IoT-enabled locker system built using NodeMCU (ESP8266). It uses various sensors, an RFID reader, and a servo motor to manage access control, detect objects, and monitor the door state. The system features a keypad for user input and an OLED display for feedback.

## Table of Contents

- [Project Overview](#project-overview)
- [Components](#components)
- [Pin Setup](#pin-setup)
- [Installation](#installation)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Future Enhancements](#future-enhancements)
- [License](#license)

## Project Overview

Seconds-Locker is designed to enhance security and accessibility by allowing users to interact with the locker using a keypad, RFID reader, and sensors. It can automatically detect the presence of objects, monitor the door state, and provide visual feedback through an OLED display.

## Components

1. **NodeMCU (ESP8266)** - Main controller.
2. **PCF8574 (I2C Expander)** - Expands GPIO capabilities for additional sensors and inputs.
3. **MFRC-522 (RFID Reader)** - Provides authentication for accessing the locker.
4. **OLED Display 128x64** - Displays system status and user prompts.
5. **PCA9685 (Servo Driver)** - Controls the servo motor for door locking.
6. **Magnetic Door Sensor (MC-38)** - Detects door state (open/closed).
7. **IR Sensor** - Detects the presence of objects inside the locker.
8. **Servo Motor** - Automatically locks and unlocks the door.
9. **Keypad** - Used for password input and box selection.

## Pin Setup

### NodeMCU Pin Configuration

- **PCF8574 (I2C Expander)**:
  - Address `0x20`: ➞ Keypad input.
  - Address `0x21`: ➞ Magnetic door sensor (MC-31).
  - Address `0x22`: ➞ IR object detection sensor.

- **MFRC-522 (RFID Reader)**:
  - `Reset (RST)` ➞ D3
  - `SS (SDA)` ➞ D4
  - `SCK` ➞ D5
  - `MISO` ➞ D6
  - `MOSI` ➞ D7

- **OLED Display (128x64, I2C Address 0x3C)**:
  - `SCK` ➞ D1
  - `SDA` ➞ D2

- **PCA9685 (Servo Driver, I2C Address 0x40)**:
  - `SCK` ➞ D1
  - `SDA` ➞ D2

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/seconds-locker.git
   cd seconds-locker
   ```

2. **Install Required Libraries**:
   Ensure you have the Arduino IDE or PlatformIO set up for NodeMCU programming, then install the following libraries:
   - **Adafruit PCA9685** for servo control
   - **Adafruit SSD1306** for the OLED display
   - **MFRC522** for RFID functionality
   - **PCF8574** for I2C expander communication
   - **Keypad** library (if using a matrix keypad)

   These libraries can be installed via the Arduino Library Manager or manually added to your project.

3. **Configure Your I2C Addresses**:
   Double-check the I2C addresses for all components in your code to ensure they match your hardware setup.

4. **Upload the Code**:
   Connect your NodeMCU to your computer and upload the code using the Arduino IDE or PlatformIO.

## Usage

1. **Power Up the System**: Connect the NodeMCU to a power source.
2. **Authenticate with RFID**: Use an RFID card to authenticate and access the locker.
3. **Enter Password on Keypad**: Input the password or select a box using the keypad.
4. **Monitor the OLED Display**: Follow prompts on the OLED display to interact with the locker.
5. **Object and Door Monitoring**: The system automatically detects objects inside and monitors the door state, locking or unlocking accordingly.

## Troubleshooting

- **Display Not Working**: Check connections to the OLED and ensure the correct I2C address (`0x3C`) is set in the code.
- **RFID Not Responding**: Verify connections to the MFRC-522 and check that the `RST` and `SDA` pins are correctly mapped in the software.
- **Servo Not Moving**: Ensure the PCA9685 is correctly connected to the I2C bus and that power supply specifications match the servo requirements.
- **Incorrect Sensor Readings**: Check the wiring of the PCF8574 and ensure the addresses correspond correctly to each sensor.

## Future Enhancements

- **Remote Access**: Add Wi-Fi functionality for remote monitoring and control via a web interface.
- **Camera Integration**: Incorporate a camera for object verification or for capturing intruder images.
- **Advanced User Interface**: Develop a more interactive UI for the OLED to display additional information like logs or system diagnostics.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.