#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Common settings
#define FIRMWARE_VERSION "1.0.4"
#define DEVICE_ID "xxxxxx"
#define DEVICE_SECRET "xxxxxx"
#define DEVICE_TYPE "ESP32"
#define LOCKER_DOORS_NUM 2
#define DOOR_OPEN_TIMEOUT 30000
#define DOOR_CLOSE_TIMEOUT 10000

// Buzzer settings
constexpr uint8_t BUZZ_PIN = x;

// WiFi settings
#define WIFI_SSID "xxxxxx"
#define WIFI_PASSWORD "xxxxxx"

// WebSocket settings
#define SERVER_HOST "192.168.x.x"
#define SOCKET_PORT 3001
#define SOCKET_PATH "/socket.io/?transport=websocket"

// Security settings
#define USE_SSL false
#define SSL_FINGERPRINT "XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX"
#define JWT_SECRET "xxxxxx" // Must match server's JWT_SECRET
#define INITIAL_AUTH_TOKEN "xxxxxx" // Pre-generated JWT token

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

#endif // CONFIG_H