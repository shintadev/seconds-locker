#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PWMServoDriver.h>
#include <I2CKeyPad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <memorysaver.h>

// WiFi credentials
const char* ssid = "********";
const char* password = "********";

// REST API endpoint
const char* apiEndpoint = "http://********";

// Websocket setup
WebSocketsClient webSocket;

const char* serverIp = "192.168.x.x";
#define WEBSOCKET_PORT 3001

// PCA9685 setup
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_MIN 150 // Min pulse length out of 4096
#define SERVO_MAX 600 // Max pulse length out of 4096

// PCF8574 setup
#define PCF8574_ADDRESS 0x21

// OLED setup
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Keypad setup
#define KEYPAD_ADDRESS 0x20
I2CKeyPad keyPad(KEYPAD_ADDRESS);

char keymap[19] = "123A456B789C*0#DNF";     // ... NoKey  Fail }

// MFRC522 setup
constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

// Other variables
int failCount = 0;
String tag;

struct VerificationResult {
  bool isSuccess;
  String response;
  int code;
};

struct ApiResponse {
  String status;
  String salary;
  String age;
  int id;
};

void setup() {
  Serial.begin(115200);

  // Initialize SPI bus
  SPI.begin();

  // Initialize I2C communication
  Wire.begin();
  Wire.setClock(400000);

  // Initialize PCA9685//////////////////////////////////
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz

  // Initialize PCF8574
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(0xFF); // Set all pins to high (inputs)
  Wire.endTransmission();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Initialize MFRC522
  rfid.PCD_Init(); 

  // Initialize keypad
  if (keyPad.begin() == false) {
    display.setCursor(0, 0);
    display.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
    display.display();

    while (keyPad.begin() == false) {
      Serial.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
      delay(1000);
    }
  }
  keyPad.loadKeyMap(keymap);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Keypad connected.");
  display.display();
  Serial.println("\nKeypad connected.\n");
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Connecting to WiFi");
    display.display();
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      display.print(".");
      display.display();
      Serial.println("Connecting to WiFi...");
    }
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connected to WiFi");
  display.display();
  Serial.println("\nConnected to WiFi");
  delay(1000);

  // Initialize WebSocket connection
  webSocket.beginSocketIOSSLWithCA(serverIp, WEBSOCKET_PORT, "/");
  webSocket.onEvent(webSocketEvent);

  webSocket.setReconnectInterval(5000);
}

void loop() {
  // Waiting any key pressed to wake
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Press any key to \nstart...");
  display.display();
  Serial.println("\nPress any key to start...");
  while(keyPad.getChar() == 'N') {
    if (rfid.PICC_IsNewCardPresent() && readRFID()) {
      //Admin mode
    }
    yield(); // Allow other processes to run
  }
  delay(200);

  // Read keypad input with a timeout of 20 seconds
  char buffer[20];
  char* otp = readKeyPad('#', buffer, sizeof(buffer), 20000);

  // Check if the string is not empty
  if (strlen(otp) > 0) {
    // Print the input string to Serial
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Input String: ");
    display.print(otp);
    display.display();
    Serial.print("\nInput String: ");
    Serial.println(otp);
    // Send the input string to the REST API to verify
    VerificationResult verifyResult = verifyCode(otp);
    if (verifyResult.isSuccess) {
      // ApiResponse apiResponse = parseJsonResponse(verifyResult.response);  // Get the ApiResponse object
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Verification succeeded!\nBox ");
      display.print(verifyResult.code);
      display.println(" unlocked.");
      display.display();
      Serial.print("\nVerification succeeded!\nBox ");
      Serial.print(verifyResult.code);
      Serial.println(" unlocked.");
      failCount = 0;
      unlockBox(pwm, verifyResult.code - 1);
      bool doorState = 1;
      while (doorState == 1) {
        doorState = checkDoorState(verifyResult.code, PCF8574_ADDRESS, verifyResult.code - 1);
      }
      uint32_t start = millis();

      while (doorState == 0 && millis() - start < 30000) {
        doorState = checkDoorState(verifyResult.code, PCF8574_ADDRESS, verifyResult.code - 1);
        checkObject(verifyResult.code, PCF8574_ADDRESS, verifyResult.code - 1, verifyResult.code + 3);

        delay(100);
        yield();
      }

      display.clearDisplay();
      display.setCursor(0, 0);

      if (doorState == 0) {
        display.print("Time out. ");
        display.display();
        Serial.print("Time out. ");
      }

      display.print("Locking the box.");
      display.display();
      Serial.println("Locking the box.");

      lockBox(pwm, verifyResult.code - 1);
      
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Wrong otp!");
      display.display();
      Serial.println("\nWrong otp!");
      failCount++;
    }
  }
  if (failCount >= 3) {
    // warning (send noti, lock this box, ring alarm...)
  }

  delay(3000); // Adjust as necessary
}

void displayMessage(const char* message) {
  display.clearDisplay();
  display.setCursor(0, 0);   // Start at top-left corner
  display.println(message);  // Print the message
  display.display();         // Display the message
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected!");
      displayMessage("Disconnected!");
      break;
    case WStype_CONNECTED:
      webSocket.sendTXT("Hello Server");
      Serial.println("Connected to server");
      displayMessage("Connected to server");
      break;
    case WStype_TEXT:
      Serial.printf("Received command: %s\n", payload);
      String command = String((char*)payload);

      // Display received command on OLED
      displayMessage(command.c_str());

      // Example: check and execute specific command
      if (command.indexOf("toggle_led") != -1) {
        Serial.println("Toggling LED!");
        // Add your action here, e.g., toggle an LED
      }
      break;
  }
}
bool readRFID() {
  tag = "";
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    if (tag == "991711115") {
      Serial.println("Em chào đại ca");
      return true;
    } else {
      Serial.println("Mày là thằng nào???");
    }
  }
  return false;
}

char* readKeyPad(char until, char* buffer, uint8_t length, uint16_t timeout) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Input your OTP: ");
  display.display();
  Serial.print("Input your OTP: ");
  uint8_t bufferIndex = 0;
  uint32_t start = millis();
  char key = '\0';
  const uint32_t debounceDelay = 200; // Adjust debounce delay as needed
  uint32_t lastKeyPressTime = 0;

  // Clear the buffer
  buffer[0] = '\0';

  while (millis() - start < timeout) {
    key = keyPad.getChar(); // Get input character

    if (key != 'N' && key != 'F') {
      uint32_t currentKeyPressTime = millis();

      // Check if debounce time has passed
      if (currentKeyPressTime - lastKeyPressTime > debounceDelay) {
        if (key == until) {
          buffer[bufferIndex] = '\0'; // Null-terminate the string
          return buffer;  // Return the input string
        } else if (key == '*') {
          // Clear the input buffer if '*' is pressed
          display.clearDisplay();
          display.setCursor(0, 0);
          display.print("Input your OTP: ");
          display.display();
          bufferIndex = 0;
          buffer[0] = '\0';
        } else if (bufferIndex < length - 1) {
          // Append the key to the buffer if space allows
          buffer[bufferIndex++] = key;
          buffer[bufferIndex] = '\0'; // Keep the string null-terminated
          display.print(key);
          display.display();
          Serial.print(key);
        }
        lastKeyPressTime = currentKeyPressTime;
      }
    }

    yield(); // Allow other processes to run
  }

  buffer[bufferIndex] = '\0'; // Ensure the buffer is null-terminated
  return buffer;  // Return buffer if timeout occurs
}

VerificationResult verifyCode(String otp) {
  VerificationResult result;
  result.isSuccess = false;
  result.response = "";
  result.code = 0;
  switch (otp.charAt(0)) {
    case '1':
      result.code = 1;
      break;
    case '2':
      result.code = 2;
      break;
    case '3':
      result.code = 3;
      break;
    case '4':
      result.code = 4;
      break;
    case 'A':
      result.code = 1;
      break;
    case 'B':
      result.code = 2;
      break;
    case 'C':
      result.code = 3;
      break;
    case 'D':
      result.code = 4;
      break;
    default:
      result.code = random(4);
  }

  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Reconnecting to WiFi");
    display.display();
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    
    for (int i = 0; i < 10; i++) { // Try reconnecting 10 times
      if (WiFi.status() == WL_CONNECTED) {
        display.print(".");
        display.display();
        Serial.println("Reconnected!");
        break;
      }
      delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Failed to reconnect to WiFi.");
      display.display();
      Serial.println("Failed to reconnect to WiFi.");
      return result; 
    }
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("OTP verifying...");
  display.display();
  
  WiFiClient client;
  HTTPClient http;

  http.begin(client, apiEndpoint);
  http.addHeader("Content-Type", "application/json");

  // Create the POST request payload
  String payload = "name=";
  payload += otp;
  payload += "&salary=123&age=23";

  int httpResponseCode = http.POST(payload); // Send the POST request

  // Get the response content
  result.response = http.getString();
  Serial.printf("HTTP Response code: %d\nResponse: %s\n", httpResponseCode, result.response.c_str());

  // Check the HTTP response code
  if (httpResponseCode == 200) {
    result.isSuccess = true;
    Serial.println("Data sent successfully!");
  } else {
    Serial.print("Failed to send data!\nCode ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return result;
}

bool checkDoorState(int boxNumber, int pcf8574Addr, int pcfPin) {
  Wire.requestFrom(pcf8574Addr, 1);
  if (Wire.available()) {
    uint8_t state = Wire.read();
    // Serial.print("Raw PCF8574 state: ");
    // Serial.println(state, BIN); // Print the raw byte in binary form for debugging

    bool pinState = (state & (1 << pcfPin)) == 0; // Check the specific pin state
    Serial.print("Door state of Box ");
    Serial.print(boxNumber);
    Serial.print(": ");
    Serial.println(pinState);

    return pinState;
  }
  return false;
}

bool checkObject(int boxNumber, int pcf8574Addr, int pcfPinX, int pcfPinY) {
  Wire.requestFrom(pcf8574Addr, 1);
  if (Wire.available()) {
    uint8_t state = Wire.read();
    // Serial.print("Raw PCF8574 state: ");
    // Serial.println(state, BIN); // Print the raw byte in binary form for debugging

    bool pinStateX = (state & (1 << pcfPinX)) == 0; // Check the specific pin state
    bool pinStateY = (state & (1 << pcfPinY)) == 0; // Check the specific pin state
    Serial.print("Object detect state of Box ");
    Serial.print(boxNumber);
    Serial.print(": ");
    Serial.println(pinStateX || pinStateY);

    return pinStateX || pinStateY;
  }
  return false;
}

void unlockBox(Adafruit_PWMServoDriver &pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(0)); // Unlock position
}

void lockBox(Adafruit_PWMServoDriver &pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(90)); // Lock position
}

int angleToPulse(int angle) {
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  return pulse;
}

ApiResponse parseJsonResponse(const String jsonString) {
  ApiResponse apiResponse;

  // Create a StaticJsonDocument with an appropriate size (adjust the size based on your JSON structure)
  JsonDocument doc;

  // Parse the JSON response
  DeserializationError error = deserializeJson(doc, jsonString);

  // Check for errors in parsing
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return apiResponse;
  }

  // Extract values from the JSON object and store them in the ApiResponse struct
  apiResponse.status = doc["status"].as<String>();
  JsonObject data = doc["data"];
  apiResponse.salary = data["salary"].as<String>();
  apiResponse.age = data["age"].as<String>();
  apiResponse.id = data["id"].as<int>();

  return apiResponse; // Return the filled ApiResponse object
}

