#include "WebSocketHandlers.h"
#include "../Locker_Setup.h"
#include "LockerOperations.h"
#include "SerialCommunication.h"
#include "TokenManager.h"

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
    writeSerial2("verifyStatus;success");
    delay(2000);
    openDoor(doc["data"]["lockerDoorId"].as<String>());
  } else {
    Serial.println("Code verification failed. Wrong otp!");
    writeSerial2("verifyStatus;failed");
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
