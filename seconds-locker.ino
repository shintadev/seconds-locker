#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PWMServoDriver.h>
#include <EEPROM.h>
#include <ESP8266Ping.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <I2CKeyPad.h>
#include <memorysaver.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WebSocketsClient.h>
#include <Wire.h>

// Locker setup
const String LOCKER_ID = "********"; 
const int LOCKERS_NUM = xxx; // Adjust as locker doors number
String token = "";
unsigned long lastHeartbeat = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastWebSocketAttempt = 0;

// EEPROM setup
#define EEPROM_SIZE 512
#define TOKEN_ADDRESS 0
#define TOKEN_MAX_LENGTH 256

// WiFi credentials & setup
const char* ssid = "********";
const char* password = "********";
char* backup_ssid = "********";
char* backup_password = "********";
ESP8266WiFiMulti WiFiMulti;

// WebSocket setup
WebSocketsClient webSocket;
const char* websocket_server = "192.168.x.x";
const int websocket_port = 3001;
const char* websocket_url = "/********";

// PCA9685 setup
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_MIN 150  // Min pulse length out of 4096
#define SERVO_MAX 600  // Max pulse length out of 4096

// PCF8574 setup
#define PCF8574_ADDRESS_1 0x21
#define PCF8574_ADDRESS_2 0x22

// OLED setup
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Keypad setup
#define KEYPAD_ADDRESS 0x20
I2CKeyPad keyPad(KEYPAD_ADDRESS);

char keymap[19] = "123A456B789C*0#DNF";  // ... NoKey  Fail }

// MFRC522 setup
constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;

MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;

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
      Serial.printf("[WSc] Disconnected!\n");
      connectionState = DISCONNECTED;
      break;
    case WStype_CONNECTED:
      {
        Serial.printf("[WSc] Connected to url: %s\n", payload);
        connectionState = CONNECTED;
        webSocket.sendTXT("Hello from locker" + LOCKER_ID);
        if (token.length() > 0) {
          authenticate();
        } else {
          registerLocker();
        }
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
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println("Starting...");

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

  // Initialize OLED display
  setupOLED();

  // Initialize MFRC522
  rfid.PCD_Init();

  // Initialize keypad
  setupKeypad();

  // Initialize WiFi
  WiFiMulti.addAP(ssid, password);

  // Initialize WebSocket connection
  webSocket.begin(websocket_server, websocket_port, websocket_url);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  connectionState = CONNECTING_WIFI;
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
          Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
          connectionState = CONNECTING_WEBSOCKET;
        } else {
          Serial.println("WiFi not connected");
        }
      }
      break;

    case CONNECTING_WEBSOCKET:
      if (currentMillis - lastWebSocketAttempt > 5000) {
        lastWebSocketAttempt = currentMillis;
        Serial.printf("Attempting to connect to WebSocket server: %s:%d%s\n", websocket_server, websocket_port, websocket_url);
        pingServer();
        webSocket.begin(websocket_server, websocket_port, websocket_url);
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
      if (currentMillis - lastHeartbeat > 10000) {
        lastHeartbeat = currentMillis;
        sendHeartbeat();
      }

      // Your main locker logic here
      handleLockerOperations();
      break;
  }
  webSocket.loop();
  yield();
}

void pingServer() {
  IPAddress serverIP;
  if (WiFi.hostByName(websocket_server, serverIP)) {
    Serial.print("Pinging ");
    Serial.print(websocket_server);
    Serial.print(" [");
    Serial.print(serverIP);
    Serial.println("]");

    if (Ping.ping(serverIP)) {
      Serial.println("Server is reachable");
    } else {
      Serial.println("Server is not reachable");
    }
  } else {
    Serial.println("Could not resolve hostname");
  }
}

void registerLocker() {
  if (connectionState != CONNECTED) return;

  DynamicJsonDocument doc(256);
  doc["event"] = "register";
  doc["data"]["lockerId"] = LOCKER_ID;
  JsonArray lockerDoors = doc["data"]["lockerDoors"].to<JsonArray>();

  for (int i = 1; i <= LOCKERS_NUM; i++) {
    JsonObject door = lockerDoors.createNestedObject();
    door["id"] = LOCKER_ID + "-" + String(i);
  }

  String output;
  serializeJson(doc, output);

  Serial.println("Sending register: " + output);
  webSocket.sendTXT(output);
  connectionState = AUTHENTICATING;
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

  DynamicJsonDocument doc(256);
  doc["event"] = "authenticate";
  doc["data"]["token"] = token;
  doc["data"]["lockerId"] = LOCKER_ID;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending authenticate: " + output);
  webSocket.sendTXT(output);
  connectionState = AUTHENTICATING;
}

void sendHeartbeat() {
  if (connectionState != AUTHENTICATED) return;

  DynamicJsonDocument doc(128);
  doc["event"] = "heartbeat";
  doc["data"]["lockerId"] = LOCKER_ID;
  doc["data"]["timestamp"] = millis();

  String output;
  serializeJson(doc, output);

  Serial.println("Sending heartbeat: " + output);
  webSocket.sendTXT(output);
}

void handleMessage(uint8_t* payload, size_t length) {
  String message = String((char*)payload);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
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

void handleRegisterResult(const DynamicJsonDocument& doc) {
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

void handleAuthResult(const DynamicJsonDocument& doc) {
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

void handleVerifyCodeResult(const DynamicJsonDocument& doc) {
  bool success = doc["data"]["success"];
  if (success) {
    Serial.println("Code verified successfully");
    displayMessage("Verification succeeded!\nBox ");
    display.print(doc["data"]["lockerDoorId"].as<int>());
    display.println(" unlocked.");
    display.display();
    Serial.print("\nVerification succeeded!\nBox ");
    Serial.print(doc["data"]["lockerDoorId"].as<int>());
    Serial.println(" unlocked.");
    openDoor(doc["data"]["lockerDoorId"].as<String>());
  } else {
    Serial.println("Code verification failed");
    displayMessage("Wrong otp!");
    Serial.println("\nWrong otp!");
    failCount++;
  }
}

void handleCommand(const DynamicJsonDocument& doc) {
  String command = doc["data"]["command"];
  String doorId = doc["data"]["lockerId"];
  if (command == "open") {
    openDoor(doorId);
    // After opening the door, send a success message
    DynamicJsonDocument doc(256);
    doc["event"] = "openCommandSuccess";
    doc["data"]["lockerId"] = LOCKER_ID;
    doc["data"]["success"] = true;

    String output;
    serializeJson(doc, output);

    Serial.println("Sending openCommandSuccess: " + output);
    webSocket.sendTXT(output);
  }
}

void openDoor(const String& doorId) {
  int doorIndex = (doorId.substring(doorId.indexOf("-") + 1)).toInt() - 1;
  Serial.println("Opening door: " + doorId);
  unlockBox(pwm, doorIndex);
  failCount = 0;

  // Limit the time to open the door in 30 seconds
  uint32_t start = millis();
  bool doorClosed = false;
  while (millis() - start < 30000 && !doorClosed) {
    if (checkDoorState(doorId.toInt(), PCF8574_ADDRESS_1, doorIndex) == 0) {
      doorClosed = true;
      break;
    }
    yield();
  }

  if (!doorClosed) {
    displayMessage("Door is still open!");
    Serial.println("Door is still open!");
    while (checkDoorState(doorId.toInt(), PCF8574_ADDRESS_1, doorIndex) == 1) {
      ringWarning(PCF8574_ADDRESS_1, 4);
      delay(1000);
      yield();
    }
  }

  lockBox(pwm, doorIndex);

  bool isObjectPresent = checkObject(doorId.toInt(), PCF8574_ADDRESS_2, doorIndex, doorIndex) == 1;
  Serial.println(isObjectPresent ? "Package detected in the box!" : "No package detected in the box!");
  sendBoxUsage(doorId, isObjectPresent);
}

void sendBoxUsage(const String& doorId, bool isObject) {
  if (connectionState != AUTHENTICATED) {
    displayMessage("ERROR\nPlease reboot or contact\nadmin for help");
    Serial.println("Locker not authenticated");
    return;
  }

  DynamicJsonDocument doc(256);
  doc["event"] = "boxUsage";
  doc["data"]["lockerId"] = LOCKER_ID;
  doc["data"]["doorId"] = doorId;
  doc["data"]["isObject"] = isObject;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending boxUsage: " + output);
  webSocket.sendTXT(output);
}

void ringWarning(int pcf8574Addr, int pcfPin) {
  if (Wire.requestFrom(pcf8574Addr, 1) && Wire.available()) {
#ifdef DEBUG
    Serial.println("Ringing the buzzer");
#endif
    for (int i = 0; i < 3; i++) {
      Wire.write(1 << pcfPin);
      delay(500);
      Wire.write(0);
      delay(500);
    }
  }
}

void handleLockerOperations() {
  // Waiting for any key pressed to wake
  displayMessage("Press any key to \nstart...");
  Serial.println("\nPress any key to start...");
  
  unsigned long startTime = millis();
  const unsigned long timeout = 10000; // 10 seconds timeout

  while (keyPad.getChar() == 'N') {
    if (rfid.PICC_IsNewCardPresent()) {
      if (readRFID()) {
        handleAdminMode();
      } else Serial.println("Invalid RFID access");
    }
    delay(200);
    yield();  // Allow other processes to run

    // Check if timeout has occurred
    if (millis() - startTime > timeout) {
      Serial.println("Timeout: continue webSocket connection");
      return;
    }
  }
  delay(200);

  // Read keypad input with a timeout of 20 seconds
  char buffer[20];
  char* otp = readKeyPad('#', buffer, sizeof(buffer), 20000);

  // Check if the string is not empty
  if (strlen(otp) > 0) {
    // Print the input string to Serial
    Serial.print("\nInput String: ");
    Serial.println(otp);
    // Send the input string to the server to verify
    sendVerifyCode(otp);
  }
  delay(200);
}

void sendVerifyCode(const char* otp) {
  if (connectionState != AUTHENTICATED) {
    displayMessage("ERROR\nPlease reboot or contact\nadmin for help");
    Serial.println("Locker not authenticated");
    return;
  }
  DynamicJsonDocument doc(256);
  doc["event"] = "verifyCode";
  doc["data"]["lockerId"] = LOCKER_ID;
  doc["data"]["otp"] = otp;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending verifyCode: " + output);
  webSocket.sendTXT(output);
  displayMessage("Verifying.");
  delay(200);
  displayMessage("Verifying..");
  delay(200);
  displayMessage("Verifying...");
  delay(400);
}

void checkWifi() {
  static unsigned long lastWiFiAttempt = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastWiFiAttempt >= 5000) {
    lastWiFiAttempt = currentMillis;

    if (WiFiMulti.run() == WL_CONNECTED) {
      Serial.println(F("WiFi connected"));
      Serial.printf_P(PSTR("IP address: %s\n"), WiFi.localIP().toString().c_str());
      connectionState = CONNECTING_WEBSOCKET;
    } else {
      displayMessage("WiFi not connected");
      Serial.println(F("WiFi not connected"));
    }
  }
}

void setupKeypad() {
  while (!keyPad.begin()) {
    displayMessage("ERROR\nPlease reboot.");
    Serial.println("ERROR: cannot communicate to keypad.\nPlease reboot.");
    delay(1000);
  }

  keyPad.loadKeyMap(keymap);
  Serial.println("Keypad connected.");
}

void setupOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Infinite loop to halt execution
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  displayMessage("Booting...");
}

void displayMessage(const char* message) {
  display.clearDisplay();
  display.setCursor(0, 0);   // Start at top-left corner
  display.println(message);  // Print the message
  display.display();         // Display the message
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
      displayMessage("WELCOME admin");
      Serial.println("Enter admin mode...");
      return true;
    } else {
      Serial.println("Illegal access attempt");
    }
  }
  return false;
}

char* readKeyPad(char until, char* buffer, uint8_t length, uint16_t timeout) {
  displayMessage("Enter your OTP: ");
  Serial.print("Enter your OTP: ");
  uint8_t bufferIndex = 0;
  uint32_t start = millis();
  const uint32_t debounceDelay = 200;
  uint32_t lastKeyPressTime = 0;

  buffer[0] = '\0';

  while (millis() - start < timeout) {
    char key = keyPad.getChar();

    if (key != 'N' && key != 'F' && millis() - lastKeyPressTime > debounceDelay) {
      lastKeyPressTime = millis();

      if (key == until) {
        buffer[bufferIndex] = '\0';
        return buffer;
      } else if (key == '*') {
        displayMessage("Enter your OTP: ");
        bufferIndex = 0;
        buffer[0] = '\0';
      } else if (bufferIndex < length - 1) {
        buffer[bufferIndex++] = key;
        buffer[bufferIndex] = '\0';
        display.print(key);
        display.display();
        Serial.print(key);
      }
    }

    yield();
  }
  displayMessage("Timeout!");
  Serial.println("Timeout!");
  delay(2000);

  // If timeout occurs, return null buffer
  buffer[0] = '\0';
  return buffer;
}

void handleAdminMode() {
  displayMessage("Admin Mode\n1:Check Doors\n2:Reset WiFi\n3:Change Backup\n4:Open Box\n5:Exit");
  Serial.println("\nAdmin Mode\n1: Check Doors\n2: Reset WiFi\n3: Change Backup WiFi\n4: Open Box\n5: Exit");

  while (true) {
    char key = keyPad.getChar();
    if (key != 'N') {
      switch (key) {
        case '1':
          checkDoors();
          break;
        case '2':
          resetWiFi();
          break;
        case '3':
          changeBackupWiFi();
          break;
        case '4':
          openBoxAdmin();
          delay(200);  // Debound delay
          break;
        case '5':
          displayMessage("Exiting Admin Mode");
          Serial.println("Exiting Admin Mode");
          delay(1000);
          displayMessage("Press any key to \nstart...");
          Serial.println("\nPress any key to start...");
          return;
        default:
          displayMessage("Invalid option");
          Serial.println("Invalid option");
          delay(1000);
          break;
      }
      displayMessage("Admin Mode\n1:Check Doors\n2:Reset WiFi\n3:Change Backup\n4:Open Box\n5:Exit");
      Serial.println("\nAdmin Mode\n1: Check Doors\n2: Reset WiFi\n3: Change Backup WiFi\n4: Open Box\n5: Exit");
    }
    yield();
  }
}

void openBoxAdmin() {
  Serial.println("Select box to open:");
  for (int i = 0; i < LOCKERS_NUM; i++) {
    String menuItem = String(i + 1) + ": Box " + String(i + 1);
    Serial.println(menuItem);
  }

  while (true) {
    displayMessage("Select box to open:");

    for (int i = 0; i < LOCKERS_NUM; i++) {
      String menuItem = String(i + 1) + ": Box " + String(i + 1);
      display.println(menuItem);
      display.display();
      Serial.println(menuItem);
    }

    char key = keyPad.getChar();
    while (key == 'N') {
      key = keyPad.getChar();
      yield();
    }
    if (key == '*') {
      displayMessage("Returning to Admin Menu");
      Serial.println("Returning to Admin Menu");
      return;
    }

    int boxNum = key - '1';
    if (boxNum >= 0 && boxNum < LOCKERS_NUM) {
      String doorId = LOCKER_ID + "-" + String(boxNum + 1);
      delay(200);
      openDoorAdmin(doorId);
    } else {
      displayMessage("Invalid box number");
      Serial.println("Invalid box number");
      delay(1000);
    }

    yield();
  }
}

void openDoorAdmin(const String& doorId) {
  int doorIndex = (doorId.substring(doorId.indexOf("-") + 1)).toInt() - 1;
  Serial.println("Opening door: " + doorId);
  unlockBox(pwm, doorIndex);
  delay(2000);
  lockBox(pwm, doorIndex);
}

void checkDoors() {
  displayMessage("Checking Doors...");
  Serial.println("Checking Doors...");
  for (int i = 0; i < LOCKERS_NUM; i++) {
    bool doorState = checkDoorState(i + 1, PCF8574_ADDRESS_1, i);
    bool objectPresent = checkObject(i + 1, PCF8574_ADDRESS_2, i, i);

    String status = "Door " + String(i + 1) + ": ";
    status += doorState ? "Open" : "Closed";
    status += ", ";
    status += objectPresent ? "Occupied" : "Empty";

    displayMessage(status.c_str());
    Serial.println(status);
    delay(1000);
  }
}

void changeBackupWiFi() {
  // Enter backup WiFi credentials
  // Enter ssid
  displayMessage("Enter SSID: ");
  Serial.println("Enter SSID: ");
  char ssid[32];
  backup_ssid = readKeyPad('#', ssid, sizeof(ssid), 10000);
  delay(200);
  // Enter password
  displayMessage("Enter Password: ");
  Serial.println("Enter Password: ");
  char password[64];
  delay(200);
  backup_password = readKeyPad('#', password, sizeof(password), 10000);
  delay(200);

  // Clear stored WiFi credentials
  WiFi.disconnect(true);
  delay(1000);

  // Save the new WiFi credentials
  WiFiMulti.addAP(backup_ssid, backup_password);
  checkWifi();
}

void resetWiFi() {
  displayMessage("Resetting WiFi...");
  Serial.println("Resetting WiFi...");
  // Clear stored WiFi credentials
  WiFi.disconnect(true);
  delay(1000);

  // Restart the ESP8266
  ESP.restart();
}

JsonDocument parseJson(const String jsonString) {
  // Create a StaticJsonDocument with an appropriate size (adjust the size based on your JSON structure)
  JsonDocument doc;

  // Parse the JSON response
  DeserializationError error = deserializeJson(doc, jsonString);

  // Check for errors in parsing
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return doc;
  }

  return doc;
}

bool checkDoorState(int boxNumber, int pcf8574Addr, int pcfPin) {
  if (Wire.requestFrom(pcf8574Addr, 1) && Wire.available()) {
    uint8_t state = Wire.read();
    bool pinState = !(state & (1 << pcfPin));

#ifdef DEBUG
    Serial.printf("Door state of Box %d: %s\n", boxNumber, pinState ? "Open" : "Closed");
#endif

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
  pwm.setPWM(servoChannel, 0, angleToPulse(0));  // Unlock position
}

void lockBox(Adafruit_PWMServoDriver& pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(90));  // Lock position
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
