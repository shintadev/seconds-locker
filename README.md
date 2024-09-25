# Seconds-Locker

Seconds-Locker is a secure, IoT-enabled locker system built using NodeMCU (ESP8266) and ESP32 board. It uses various sensors, RFID readers, and servo motors to manage access control, detect objects, and monitor the door state. The system features a keypad for user input and a TFT touch display for user interface.

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

Seconds-Locker is designed as sub-project of the Seconds platform to enhance security and accessibility by allowing users to interact with the locker using a keypad, RFID reader, and sensors. It can automatically detect the presence of objects, monitor the door state, and provide visual UI through a TFT touch display, as well as communicate with the Seconds server.

## Components

1. **NodeMCU (ESP8266)** - Main controller.
2. **ESP32** - Secondary controller (Handles user interface and display interactions).
3. **PCF8574 (I2C Expander)** - Expands GPIO capabilities for additional sensors and inputs.
4. **MFRC-522 (RFID Reader)** - Provides authentication for accessing Admin mode.
5. **TFT SPI ILI9488 3.5" Touch** - Displays UI.
6. **PCA9685 (Servo Driver)** - Controls the servo motor for door locking.
7. **Servo Motor** - Automatically locks and unlocks the door.
8. **Magnetic Door Sensor (MC-38)** - Detects door state (open/closed).
9. **IR Sensor** - Detects the presence of objects inside the locker (occupied/empty).
10. **Keypad** - Used for password input and box selection.
11. **Buzzer** - Used for warning function.

## Pin Setup

### NodeMCU Pin Configuration

- **PCF8574 (I2C Expander)**:
  - Address `0x21`: ➞ Magnetic door sensor (MC-38).
  - Address `0x22`: ➞ IR object detection sensor.

- **MFRC-522 (RFID Reader)**:
  - `Reset (RST)` ➞ D3
  - `SS (SDA)` ➞ D4
  - `SCK` ➞ D5
  - `MISO` ➞ D6
  - `MOSI` ➞ D7

- **PCA9685 (Servo Driver, I2C Address 0x40)**:
  - `SCK` ➞ D1
  - `SDA` ➞ D2

- **Buzzer**:
  - D8

### ESP32 Pin Configuration

- **TFT SPI ILI9488 3.5" Touch**
  - `CS (Chip select)` ➞ P15
  - `RESET` ➞ P4
  - `DC/RS` ➞ P2
  - `MOSI` ➞ P23
  - `SCK` ➞ P18
  - `LED` ➞ 5V
  - `T_CLK` ➞ P18
  - `T_CS` ➞ P5
  - `T_DIN` ➞ P23
  - `T_DO` ➞ P19

- **Keypad**
  - Connect to PCF8574 using this library.

- **UART communication**
  - P16 as `RX`
  - P17 as `TX`

> NodeMCU TX must be connect to ESP32 RX, and vice versa.
> DON'T connect UART when upload code.

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/seconds-locker.git
   cd seconds-locker
   ```

2. **Install Required Libraries**:
   Ensure you have the Arduino IDE or PlatformIO set up for NodeMCU programming, then install the libraries listed in the code:

   These libraries can be installed via the Arduino Library Manager or manually added to your project.

3. **Configure Your I2C Addresses**:
   Double-check the I2C addresses for all components in your code to ensure they match your hardware setup.

4. **Upload the Code**:
   Connect your NodeMCU to your computer and upload the code using the Arduino IDE or PlatformIO. Do the same with ESP32 parts.

> Ensure you have a functional WebSocket server set up before using the code, as it relies on this connection.

## Usage

1. **Power Up the System**: Connect the NodeMCU and ESP32 to a power source.
2. **Authenticate with RFID**: Use an RFID card to authenticate and access the admin mode.
3. **Interact with the Display**: Follow prompts on the touch display to interact with the locker.
4. **Enter Code on Keypad**: Input the code using the keypad.
5. **Object and Door Monitoring**: The system automatically detects objects inside and monitors the door state, locking or unlocking accordingly.

## Troubleshooting

- **General Issues**: when encountering an issue, you can first try these self-troubleshooting steps
  - Disconnect and reconnect cables.
  - Uninstall and reinstall board drivers.
  - Check the wiring.
  - Ensure I2C connection by using Check_i2c code

## Future Enhancements

- **Advanced User Interface**: Develop a more interactive UI for the TFT display additional information and better UX.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.