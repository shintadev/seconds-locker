# Seconds-Locker

Seconds-Locker is a secure, IoT-enabled locker system built using two ESP32 boards. It uses various sensors, RFID readers, and servo motors to manage access control, detect objects, and monitor the door state. The system features a keypad for user input and a TFT touch display for user interface.

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

Seconds-Locker is designed as a sub-project of the Seconds platform to enhance security and accessibility by allowing users to interact with the locker using a keypad, RFID reader, and sensors. It can automatically detect the presence of objects, monitor the door state, and provide visual UI through a TFT touch display, as well as communicate with the Seconds server.

## Components

1. **ESP32 (Main Controller)** - Handles core locker operations and sensor interactions.
2. **ESP32 (Display Controller)** - Manages user interface and display interactions.
3. **PCF8574 (I2C Expander)** - Expands GPIO capabilities for additional sensors and inputs.
4. **MFRC-522 (RFID Reader)** - Provides authentication for accessing Admin mode.
5. **TFT SPI ILI9488 3.5" Touch** - Displays UI.
6. **PCA9685 (Servo Driver)** - Controls the servo motors for door locking.
7. **Servo Motors** - Automatically lock and unlock the doors.
8. **Magnetic Door Sensors (MC-38)** - Detect door states (open/closed).
9. **IR Sensors** - Detect the presence of objects inside the lockers (occupied/empty).
10. **Keypad** - Used for password input and box selection.
11. **Buzzer** - Used for warning function.

## Pin Setup

### ESP32 (Main Controller) Pin Configuration

- **PCF8574 (I2C Expander)**:
  - Address `0x21`: ➞ Magnetic door sensors (MC-38).
  - Address `0x22`: ➞ IR object detection sensors.

- **MFRC-522 (RFID Reader)**:
  - `Reset (RST)` ➞ GPIO 4
  - `SS (SDA)` ➞ GPIO 5
  - `SCK` ➞ GPIO 18
  - `MISO` ➞ GPIO 19
  - `MOSI` ➞ GPIO 23

- **PCA9685 (Servo Driver, I2C Address 0x40)**:
  - `SCL` ➞ GPIO 22
  - `SDA` ➞ GPIO 21

- **Buzzer**:
  - GPIO 15

### ESP32 (Display Controller) Pin Configuration

- **TFT SPI ILI9488 3.5" Touch**
  - `CS (Chip select)` ➞ GPIO 15
  - `RESET` ➞ GPIO 4
  - `DC/RS` ➞ GPIO 2
  - `MOSI` ➞ GPIO 23
  - `SCK` ➞ GPIO 18
  - `LED` ➞ 3.3V
  - `T_CLK` ➞ GPIO 18
  - `T_CS` ➞ GPIO 5
  - `T_DIN` ➞ GPIO 23
  - `T_DO` ➞ GPIO 19

- **Keypad**
  - Connect to PCF8574 using the [I2CKeyPad library](https://github.com/RobTillaart/I2CKeyPad).

- **UART communication**
  - GPIO 16 as `RX`
  - GPIO 17 as `TX`

> Main ESP32 TX must be connected to Display ESP32 RX, and vice versa.
> DON'T connect UART when uploading code.

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/seconds-locker.git
   cd seconds-locker
   ```

2. **Install Required Libraries**:
   Ensure you have the Arduino IDE or PlatformIO set up for ESP32 programming, then install the following libraries:
   - TFT_eSPI
   - TFT_eWidget
   - I2CKeyPad
   - Wire
   - Adafruit_PWMServoDriver

   These libraries can be installed via the Arduino Library Manager or manually added to your project.

3. **Configure Your I2C Addresses**:
   Double-check the I2C addresses for all components in your code to ensure they match your hardware setup.

4. **Upload the Code**:
   Connect both ESP32 boards to your computer and upload the respective code using the Arduino IDE or PlatformIO.

> Ensure you have a functional WebSocket server set up before using the code, as it relies on this connection.

## Usage

1. **Power Up the System**: Connect both ESP32 boards to a power source.
2. **Language Selection**: Choose your preferred language (EN or VN) on the initial screen.
3. **Verification Method**: Select either OTP or QR Code verification method.
4. **Enter Code or Scan QR**: Input the OTP using the keypad or scan the QR code as per your selection.
5. **Admin Mode**: Use an RFID card to authenticate and access the admin mode.
6. **Admin Functions**: In admin mode, you can check door status, open boxes, reset the system, or exit.
7. **Object and Door Monitoring**: The system automatically detects objects inside and monitors the door state, locking or unlocking accordingly.

## Troubleshooting

- **General Issues**: When encountering an issue, you can first try these self-troubleshooting steps:
  - Disconnect and reconnect cables.
  - Check the wiring.
  - Ensure I2C connection by using I2C scanner code.
- **Display Issues**: If the display is not responding, check the touch calibration and TFT_eSPI library configuration.
- **Communication Problems**: Verify the UART connections between the two ESP32 boards and ensure the baud rates match.

## Future Enhancements

- **Advanced User Interface**: Develop a more interactive UI for the TFT display with additional information and better UX.
- **Remote Management**: Implement remote management capabilities through a web interface or mobile app.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.