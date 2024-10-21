#ifndef LOCKER_SETUP_H
#define LOCKER_SETUP_H

#include <Arduino.h>

// Common settings
#define LOCKER_ID "xxxxxxxx"
#define LOCKER_DOORS_NUM x
#define DOOR_OPEN_TIMEOUT 30000
#define DOOR_CLOSE_TIMEOUT 10000

// Buzzer settings
constexpr uint8_t BUZZ_PIN = 15;

// WiFi settings
#define WIFI_SSID "xxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxx"

// WebSocket settings
#define WEBSOCKET_SERVER "192.168.x.x"
#define WEBSOCKET_PORT x
#define WEBSOCKET_URL "/"

// QR Code settings
#define QR_URL "xxxxxx"

// Servo settings
#define SERVO_MIN 150
#define SERVO_MAX 600

// I2C addresses
#define PCF8574_ADDRESS_1 0x21
#define PCF8574_ADDRESS_2 0x22

// Display settings
#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

// Keypad settings
#define KEYPAD_ADDRESS 0x20

// Button dimensions
#define BUTTON_W 150
#define BUTTON_H 50

// Serial communication
#define SERIAL_BAUD_RATE 115200

// EEPROM settings
#define EEPROM_SIZE 512
#define TOKEN_ADDRESS 0
#define TOKEN_MAX_LENGTH 256

#endif // LOCKER_SETUP_H