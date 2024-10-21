#include "../Locker_Setup.h"
#include "WebSocketHandlers.h"
#include "LockerOperations.h"
#include "SerialCommunication.h"
#include "RFIDHandler.h"
#include "TokenManager.h"
// #include <ArduinoJson.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WebSocketsClient.h>
#include <Wire.h>

// Locker setup
String token = "";
bool initalWebSocketAttempt = true;

// WiFi credentials & setup
WiFiMulti WiFiMulti;

// WebSocket setup
WebSocketsClient webSocket;

// PCA9685 setup
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// MFRC522 setup
constexpr uint8_t RST_PIN = 4;
constexpr uint8_t SS_PIN = 5;

MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;

// Other variables
uint32_t lastHeartbeat = 0;
uint32_t lastWebSocketAttempt = 0;
uint32_t lastSerial2Write = 0;
uint32_t lastSerial2Read = 0;
int failCount = 0;

ConnectionState connectionState = DISCONNECTED;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("\nStarting...");

  // Uncomment the following line to clear the EEPROM (run once, then comment it out again)
  // clearEEPROM();

  loadToken();

  // Initialize SPI bus
  SPI.begin();

  // Initialize I2C communication
  Wire.begin();
  Wire.setClock(400000);

  // Initialize PCA9685
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz

  // Initialize PCF8574
  Wire.beginTransmission(PCF8574_ADDRESS_1);
  Wire.write(0xFF);  // Set all pins to high (inputs)
  Wire.endTransmission();

  Wire.beginTransmission(PCF8574_ADDRESS_2);
  Wire.write(0xFF);  // Set all pins to high (inputs)
  Wire.endTransmission();

  // Initialize MFRC522
  rfid.PCD_Init();

  // Initialize buzzer
  pinMode(BUZZ_PIN, OUTPUT);

  // Initialize WiFi
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  // Initialize WebSocket connection
  webSocket.begin(WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  connectionState = CONNECTING_WIFI;
  delay(1000);
  // Ring a new melody to announce setup completion
  digitalWrite(BUZZ_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZ_PIN, LOW);
  delay(50);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZ_PIN, LOW);
  delay(100);
  Serial.println("Setup finished");
}

void loop() {
  uint32_t currentMillis = millis();

  if (connectionState != AUTHENTICATED && currentMillis - lastSerial2Write >= 5000) {
    if (connectionState == CONNECTING_WEBSOCKET || connectionState == AUTHENTICATING) {
      // Add a delay before sending NOT_READY during connection attempts
      if (currentMillis - lastWebSocketAttempt > 10000) {
        writeSerial2("NOT_READY");
        lastSerial2Write = currentMillis;
      }
    } else {
      writeSerial2("NOT_READY");
      lastSerial2Write = currentMillis;
    }
  }

  switch (connectionState) {
    case DISCONNECTED:
    case CONNECTING_WIFI:
      {
        Serial.println("State: DISCONNECTED or CONNECTING_WIFI");
        Serial.println("Attempting to connect to WiFi...");
        WiFi.disconnect(true);  // Disconnect from any previous connection
        WiFi.mode(WIFI_STA);    // Set WiFi mode to station
        delay(100);
        int status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        Serial.printf("WiFi begin status: %d\n", status);

        uint32_t startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
          delay(500);
          Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("WiFi connected successfully");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          connectionState = CONNECTING_WEBSOCKET;
        } else {
          Serial.println("WiFi connection failed");
          Serial.printf("WiFi status: %d\n", WiFi.status());
        }
      }
      break;

    case CONNECTING_WEBSOCKET:
      Serial.println("State: CONNECTING_WEBSOCKET");
      if (initalWebSocketAttempt || currentMillis - lastWebSocketAttempt > 5000) {
        initalWebSocketAttempt = false;
        lastWebSocketAttempt = currentMillis;
        Serial.printf("Attempting to connect to WebSocket server: %s:%d%s\n", WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
        webSocket.begin(WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
      }

      // Add connection timeout check
      if (currentMillis - lastWebSocketAttempt > 30000) {  // 30 seconds timeout
        Serial.println("WebSocket connection attempt timed out. Retrying...");
        connectionState = DISCONNECTED;
      }
      break;

    case CONNECTED:
      {
        Serial.println("State: CONNECTED");
        if (token.length() > 0) {
          authenticate();
        } else {
          registerLocker();
        }
      }
      break;

    case AUTHENTICATING:
      {
        Serial.println("State: AUTHENTICATING");
        // Wait for authentication result
      }
      break;

    case AUTHENTICATED:
      {
        // Serial.println("State: AUTHENTICATED");
        // Send heartbeat to server once each 10 seconds
        // if (currentMillis - lastHeartbeat > 10000) {
        //   lastHeartbeat = currentMillis;
        //   sendHeartbeat();
        // }

        if (currentMillis - lastSerial2Read >= 500) {
          lastSerial2Read = currentMillis;
          handleSerial2();
        }

        if (currentMillis - lastSerial2Write >= 5000) {
          writeSerial2("READY");
          lastSerial2Write = currentMillis;
        }

        handleLockerOperations();
      }
      break;
  }

  webSocket.loop();  // Process WebSocket events
  yield();           // Allow other background tasks to run
  delay(100);
}

void enterDeepSleep() {
  Serial.println("Entering deep sleep mode");
  esp_deep_sleep_start();
}
