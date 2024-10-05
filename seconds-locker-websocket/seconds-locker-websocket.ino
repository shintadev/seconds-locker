#include "../Locker_Setup.h"
#include <ArduinoJson.h>
#include <Adafruit_PWMServoDriver.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WebSocketsClient.h>
#include <Wire.h>

// Locker setup
String token = "";
unsigned long lastHeartbeat = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastWebSocketAttempt = 0;

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

// Buzzer setup
constexpr uint8_t BUZZ_PIN = 15;

// Other variables
int failCount = 0;

enum ConnectionState {
  DISCONNECTED,
  CONNECTING_WIFI,
  CONNECTING_WEBSOCKET,
  CONNECTED,
  AUTHENTICATING,
  AUTHENTICATED
};

ConnectionState connectionState = DISCONNECTED;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!\n");
      connectionState = DISCONNECTED;
      break;
    case WStype_CONNECTED:
      {
        Serial.printf("[WSc] Connected to url: %s\n", payload);
        connectionState = CONNECTED;
        webSocket.sendTXT("Hello from locker" + String(LOCKER_ID));
      }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] Received text: %s\n", payload);
      handleMessage(payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[WSc] Received binary data\n");
      break;
    case WStype_ERROR:
      Serial.printf("[WSc] Error: %u\n", length);
      break;
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      Serial.printf("[WSc] Received fragmented data\n");
      break;
    default:
      Serial.printf("[WSc] Unhandled event type: %d\n", type);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

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
  Serial.println("Setup finished");
}

void loop() {
  unsigned long currentMillis = millis();

  switch (connectionState) {
    case DISCONNECTED:
    case CONNECTING_WIFI:
      if (currentMillis - lastWiFiAttempt > 5000) {
        lastWiFiAttempt = currentMillis;
        if (WiFiMulti.run() == WL_CONNECTED) {
          Serial.println("WiFi connected");
          connectionState = CONNECTING_WEBSOCKET;
        } else {
          Serial.println("WiFi not connected");
        }
      }
      break;

    case CONNECTING_WEBSOCKET:
      if (currentMillis - lastWebSocketAttempt > 5000) {
        lastWebSocketAttempt = currentMillis;
        // Serial.printf("Attempting to connect to WebSocket server: %s:%d%s\n", websocket_server, websocket_port, websocket_url);
        webSocket.begin(WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
      }
      webSocket.loop();  // Process WebSocket events

      // Add connection timeout check
      if (currentMillis - lastWebSocketAttempt > 30000) {  // 30 seconds timeout
        Serial.println("WebSocket connection attempt timed out. Retrying...");
        connectionState = DISCONNECTED;
      }
      break;

    case CONNECTED:
      if (token.length() > 0) {
        authenticate();
      } else {
        registerLocker();
      }
      break;

    case AUTHENTICATING:
      // Wait for authentication result
      break;

    case AUTHENTICATED:
      // Send heartbeat to server once each 10 seconds
      if (currentMillis - lastHeartbeat > 10000) {
        lastHeartbeat = currentMillis;
        sendHeartbeat();
      }

      // Main locker logic here
      handleLockerOperations();
      break;
  }
  webSocket.loop();  // Process WebSocket events
  yield();           // Allow other background tasks to run
}

void registerLocker() {
  if (connectionState != CONNECTED) return;

  JsonDocument doc;
  doc["event"] = "register";
  doc["data"]["lockerId"] = String(LOCKER_ID);
  JsonArray lockerDoors = doc["data"]["lockerDoors"].to<JsonArray>();

  for (int i = 1; i <= LOCKER_DOORS_NUM; i++) {
    JsonObject door = lockerDoors.add<JsonObject>();
    door["id"] = String(LOCKER_ID) + "-" + String(i);
  }

  String output;
  serializeJson(doc, output);

  Serial.println("Sending register: " + output);
  webSocket.sendTXT(output);
  connectionState = CONNECTED;
}

void authenticate() {
  if (connectionState != CONNECTED) return;
  if (token.length() == 0 || token.length() > TOKEN_MAX_LENGTH) {
    Serial.println("Invalid token. Clearing and re-registering.");
    token = "";
    saveToken();
    registerLocker();
    return;
  }

  JsonDocument doc;
  doc["event"] = "authenticate";
  doc["data"]["token"] = token;
  doc["data"]["lockerId"] = String(LOCKER_ID);

  String output;
  serializeJson(doc, output);

  Serial.println("Sending authenticate: " + output);
  webSocket.sendTXT(output);
  connectionState = AUTHENTICATING;
}

void sendHeartbeat() {
  if (connectionState != AUTHENTICATED) return;

  JsonDocument doc;
  doc["event"] = "heartbeat";
  doc["data"]["lockerId"] = String(LOCKER_ID);
  doc["data"]["timestamp"] = millis();

  String output;
  serializeJson(doc, output);

  Serial.println("Sending heartbeat: " + output);
  webSocket.sendTXT(output);
}

void handleMessage(uint8_t* payload, size_t length) {
  String message = String((char*)payload);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  String event = doc["event"];

  if (event == "registerResult") {
    handleRegisterResult(doc);
  } else if (event == "authResult") {
    handleAuthResult(doc);
  } else if (event == "reauthenticate") {
    connectionState = CONNECTED;
  } else if (event == "verifyCodeResult") {
    handleVerifyCodeResult(doc);
  } else if (event == "command") {
    handleCommand(doc);
  } else {
    Serial.println("Received message: " + message);
  }
}

void handleRegisterResult(const JsonDocument& doc) {
  bool success = doc["data"]["success"];
  if (success) {
    token = doc["data"]["token"].as<String>();
    if (token.length() > 0 && token.length() <= TOKEN_MAX_LENGTH) {
      Serial.println("Registration successful. Token: " + token);
      saveToken();
      connectionState = CONNECTED;
    } else {
      Serial.println("Received invalid token during registration");
      token = "";
      connectionState = DISCONNECTED;
    }
  } else {
    Serial.println("Registration failed: " + doc["data"]["error"].as<String>());
    connectionState = DISCONNECTED;
  }
}

void handleAuthResult(const JsonDocument& doc) {
  bool success = doc["data"]["success"];
  if (success) {
    String newToken = doc["data"]["token"].as<String>();
    if (newToken.length() > 0 && newToken.length() <= TOKEN_MAX_LENGTH) {
      token = newToken;
      Serial.println("Authentication successful. New token: " + token);
      saveToken();
      connectionState = AUTHENTICATED;
    } else {
      Serial.println("Received invalid token during authentication");
      token = "";
      connectionState = CONNECTED;
    }
  } else {
    Serial.println("Authentication failed: " + doc["data"]["error"].as<String>());
    connectionState = CONNECTED;
    token = "";
    saveToken();
  }
}

void handleVerifyCodeResult(const JsonDocument& doc) {
  bool success = doc["data"]["success"];
  if (success) {
    Serial.println("Code verified successfully");
    Serial.print("Box ");
    Serial.print(doc["data"]["lockerDoorId"].as<String>());
    Serial.println(" unlocked.");
    openDoor(doc["data"]["lockerDoorId"].as<String>());
  } else {
    Serial.println("Code verification failed. Wrong otp!");
    failCount++;
  }
}

void handleCommand(const JsonDocument& doc) {
  String command = doc["data"]["command"];
  String doorId = doc["data"]["doorId"];
  if (command == "open") {
    openDoor(doorId);
    // After opening the door, send a success message
    JsonDocument doc;
    doc["event"] = "openCommandSuccess";
    doc["data"]["lockerId"] = String(LOCKER_ID);
    doc["data"]["success"] = true;

    String output;
    serializeJson(doc, output);

    Serial.println("Sending openCommandSuccess: " + output);
    webSocket.sendTXT(output);
  }
}

void openDoor(const String& doorId) {
  Serial.println(doorId);
  int doorIndex = (doorId.substring(doorId.indexOf("-") + 1).toInt() - 1);
  if (!doorId) {
    Serial.println("Something wrong here at openDoor");
    return;
  }
  Serial.printf("Unlocking door: %d\n", doorIndex);
  unlockBox(pwm, doorIndex);
  failCount = 0;
  delay(1000);
  Serial.printf("Unlocked door: %d\n", doorIndex);

  // Limit the time to open the door in 30 seconds
  uint32_t start = millis();
  bool doorClosed = true;

  // Wait for the door to be opened
  while (millis() - start < 30000) {
    int doorState = checkDoorState(doorIndex + 1, PCF8574_ADDRESS_1, doorIndex);
    if (doorState == 0) {  // Assuming 0 means door is open
      doorClosed = false;
      Serial.println("Door opened.");
      Serial.println(doorId);
      break;
    }
    yield();  // Allow other background tasks to run
  }

  delay(500);

  // If the door was opened, wait for it to be closed
  if (!doorClosed) {
    start = millis();  // Reset timer for door close wait time
    while (millis() - start < 10000 && !doorClosed) {
      int doorState = checkDoorState(doorIndex + 1, PCF8574_ADDRESS_1, doorIndex);
      if (doorState == 1) {  // Assuming 1 means door is closed
        doorClosed = true;
        Serial.println("Door closed.");
        Serial.println(doorId);
        break;
      }
      yield();  // Allow other background tasks to run
    }
  }

  // If the door did not close in the given time, issue a warning
  if (!doorClosed) {
    Serial.println("Door is still open! Issuing warning.");
    while (checkDoorState(doorIndex + 1, PCF8574_ADDRESS_1, doorIndex) == 0) {  // Assuming 0 means door is open
      ringWarning();
      delay(1000);
      yield();  // Allow other background tasks to run
    }
  }

  delay(500);

  lockBox(pwm, doorIndex);
  Serial.println(doorId);

  bool isObjectPresent = checkObject(doorIndex + 1, PCF8574_ADDRESS_2, doorIndex, doorIndex) == 1;
  Serial.println(isObjectPresent ? "Package detected in the box!" : "No package detected in the box!");
  sendBoxUsage(doorId, isObjectPresent);
}

void sendBoxUsage(const String& doorId, bool isObject) {
  if (connectionState != AUTHENTICATED) {
    Serial.println("Locker not authenticated");
    return;
  }

  JsonDocument doc;
  doc["event"] = "boxUsage";
  doc["data"]["lockerId"] = String(LOCKER_ID);
  doc["data"]["doorId"] = doorId;
  doc["data"]["isObject"] = isObject;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending boxUsage: " + output);
  webSocket.sendTXT(output);
}

void ringWarning() {
#ifdef DEBUG
  Serial.println("Ringing the buzzer");
#endif
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZ_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZ_PIN, LOW);
    delay(300);
  }
}

void handleLockerOperations() {
  handleSerial2();

  if (rfid.PICC_IsNewCardPresent()) {
    if (readRFID()) {
      writeSerial2("ADMIN");
    }  // Allow other processes to run
  }
  delay(1000);
}

void handleSerial2() {
  String *payload = readSerial2();

  if (payload == nullptr) {
    Serial.println("No data received or readSerial2() returned nullptr");
    return;
  }

  if (payload[0].length() == 0) {  // Check if the payload has no data
    Serial.println("Payload is empty");
    return;
  }

  if (payload[0] == "verifyCode") {
    sendVerifyCode(payload[1].c_str());
  } else if (payload[0] == "sendQR") {
    // sendQRRequest()
  } else if (payload[0] == "checkStatus") {
    String data = "status;";
    for (int i = 0; i < LOCKER_DOORS_NUM; i++) {
      bool doorState = checkDoorState(i + 1, PCF8574_ADDRESS_1, i);
      bool objectPresent = checkObject(i + 1, PCF8574_ADDRESS_2, i, i);

      String status = "Door " + String(i + 1) + ": ";
      status += doorState ? "Closed" : "Open";
      status += ", ";
      status += objectPresent ? "Occupied" : "Empty";
      status += ";";
      data += status;
      delay(100);
    }
    delay(1000);
    writeSerial2(data);
  } else if (payload[0] == "reset") {
    // Clear stored WiFi credentials
    WiFi.disconnect(true);
    delay(1000);

    // Restart the ESP32
    ESP.restart();
  } else if (payload[0] == "openBoxAdmin") {
    delay(500);
    int doorIndex = payload[1].toInt() - 1;
    Serial.println(doorIndex);
    openDoorAdmin(doorIndex);
  } else {
    Serial.print("Crap message: ");
    Serial.println(payload[0].c_str());
  }
}

String *readSerial2() {
  static String array[10];
  int arrayIndex = 0;
  unsigned long lastDataReceived = millis();

  char buffer[128] = { 0 };  // Statical allocate buffer and initialize to zero

  while (Serial2.available() > 0 && (millis() - lastDataReceived) < 500) {  // Increase timeout to 1000 ms
    char c = Serial2.read();
    Serial.write(c);              // Echo received data for testing (optional)
    lastDataReceived = millis();  // Update last data received time

    if (c == '\n') {
      if (strlen(buffer) > 0) {  // Check if buffer is not empty
        array[arrayIndex++] = String(buffer);
        buffer[0] = '\0';  // Reset buffer
      }
      if (arrayIndex == 10) {
        Serial.println("Serial2 array overflow");
        break;
      }
      return array;  // Return collected data
    } else if (c == ';') {
      if (strlen(buffer) > 0) {  // Check if buffer is not empty
        array[arrayIndex++] = String(buffer);
        buffer[0] = '\0';  // Reset buffer
      }
    } else {
      size_t len = strlen(buffer);
      if (len < sizeof(buffer) - 1) {  // Prevent buffer overflow
        buffer[len] = c;               // Append character to buffer safely
        buffer[len + 1] = '\0';        // Null-terminate
      } else {
        Serial.println("Serial2 buffer overflow");
        break;
      }
    }
    yield();
  }

  return nullptr;  // Return null if we haven't received a complete line
}


void writeSerial2(const String& data) {
  if (!Serial2) {
    Serial.println("writeSerial2 failed");
    return;  // Ensure Serial is initialized and available
  }

  for (char c : data) {
    Serial2.write(c);
  }
  Serial2.write('\n');
}

void sendVerifyCode(const char* otp) {
  if (connectionState != AUTHENTICATED) {
    Serial.println("Locker not authenticated");
    return;
  }
  JsonDocument doc;
  doc["event"] = "verifyCode";
  doc["data"]["lockerId"] = String(LOCKER_ID);
  doc["data"]["otp"] = otp;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending verifyCode: " + output);
  webSocket.sendTXT(output);
}

bool readRFID() {
  String tag = "";
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    if (tag == "991711115") {
      Serial.println("Enter admin mode...");
      return true;
    } else {
      Serial.println("Illegal access attempt");
    }
  }
  return false;
}

void openDoorAdmin(const int doorIndex) {
  Serial.printf("Unlocking door: %d\n", doorIndex);
  unlockBox(pwm, doorIndex);
  delay(1000);
  Serial.printf("Unlocked door: %d\n", doorIndex);

  bool doorClosed = true;

  // Wait for the door to be opened
  while (doorClosed) {
    int doorState = checkDoorState(doorIndex + 1, PCF8574_ADDRESS_1, doorIndex);
    if (doorState == 0) {  // Assuming 0 means door is open
      doorClosed = false;
      Serial.println("Door opened.");
      break;
    }
    yield();  // Allow other background tasks to run
  }

  delay(500);

  // If the door was opened, wait for it to be closed
  while (!doorClosed) {
    int doorState = checkDoorState(doorIndex + 1, PCF8574_ADDRESS_1, doorIndex);
    if (doorState == 1) {  // Assuming 1 means door is closed
      doorClosed = true;
      Serial.println("Door closed.");
      break;
    }
    yield();  // Allow other background tasks to run
  }

  delay(500);

  lockBox(pwm, doorIndex);
}

// void resetWiFi() {
//   displayMessage("Resetting WiFi...");
//   // displayMessage("Resetting WiFi...");
//   // Clear stored WiFi credentials
//   WiFi.disconnect(true);
//   delay(1000);

//   // Restart the ESP8266
//   ESP.restart();
// }

bool checkDoorState(int boxNumber, int pcf8574Addr, int pcfPin) {
  if (Wire.requestFrom(pcf8574Addr, 1) && Wire.available()) {
    uint8_t state = Wire.read();
    bool pinState = !(state & (1 << pcfPin));

    return pinState;
  }
  return false;
}

bool checkObject(int boxNumber, int pcf8574Addr, int pcfPinX, int pcfPinY) {
  if (Wire.requestFrom(pcf8574Addr, 1) && Wire.available()) {
    uint8_t state = Wire.read();
    bool objectDetected = ((state & ((1 << pcfPinX) | (1 << pcfPinY))) == 0);

#ifdef DEBUG
    Serial.printf("Object detect state of Box %d: %s\n", boxNumber, objectDetected ? "Detected" : "Not Detected");
#endif

    return objectDetected;
  }
  return false;
}

void unlockBox(Adafruit_PWMServoDriver& pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(135));  // Unlock position
}

void lockBox(Adafruit_PWMServoDriver& pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(0));  // Lock position
}

int angleToPulse(int angle) {
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  return pulse;
}

void saveToken() {
  EEPROM.begin(EEPROM_SIZE);
  for (unsigned int i = 0; i < TOKEN_MAX_LENGTH; i++) {
    if (i < token.length()) {
      EEPROM.write(TOKEN_ADDRESS + i, token[i]);
    } else {
      EEPROM.write(TOKEN_ADDRESS + i, 0);  // Null-terminate the string
    }
  }
  bool success = EEPROM.commit();
  EEPROM.end();

  if (success) {
    Serial.println("Token saved to EEPROM");
  } else {
    Serial.println("Failed to save token to EEPROM");
  }
}

void loadToken() {
  EEPROM.begin(EEPROM_SIZE);
  token = "";
  bool validToken = false;
  for (int i = 0; i < TOKEN_MAX_LENGTH; i++) {
    char c = EEPROM.read(TOKEN_ADDRESS + i);
    if (c == 0) {
      validToken = true;
      break;
    }
    if (isAscii(c) && c != '{' && c != '}') {  // Exclude {} characters
      token += c;
    } else {
      Serial.println("Invalid character found in stored token. Clearing token.");
      token = "";
      break;
    }
  }
  EEPROM.end();

  if (token.length() > 0 && validToken) {
    Serial.println("Token loaded from EEPROM: " + token);
  } else {
    Serial.println("No valid token found in EEPROM");
    token = "";  // Ensure token is empty if invalid
  }
}

void clearEEPROM() {
  Serial.println("Clearing EEPROM...");
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF);  // Write 0xFF instead of 0
  }
  bool success = EEPROM.commit();
  EEPROM.end();
  if (success) {
    Serial.println("EEPROM cleared successfully");
  } else {
    Serial.println("Failed to clear EEPROM");
  }
}
