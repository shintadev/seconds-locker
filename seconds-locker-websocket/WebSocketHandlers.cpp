#include "WebSocketHandlers.h"
#include "../config.h"
#include "LockerOperations.h"
#include "SerialCommunication.h"

void setupWebSocket() {
  // Setup 'on' listen events
  webSocket.on("connect", socket_Connected);
  webSocket.on("disconnect", socket_Disconnected);
  webSocket.on("error", socket_error);
  webSocket.on("verifyCodeResult", socket_verifyCodeResult);
  webSocket.on("openDoorRequest", socket_openDoor);

  // Log event listeners setup
  Serial.println("Event listeners setup successfully.");
}

void connectWebsocket() {
  // Log WebSocket server connection attempt
  Serial.printf("Attempting to connect to WebSocket server: %s:%d%s\n", SERVER_HOST, SOCKET_PORT, SOCKET_PATH);

  // Create authentication headers
  char authHeaders[256];
  snprintf(authHeaders, sizeof(authHeaders),
           "Authorization=%s&deviceSecret=%s",
           INITIAL_AUTH_TOKEN,
           DEVICE_SECRET);

  // Add security parameters to connection path
  char fullPath[384];
  snprintf(fullPath, sizeof(fullPath),
           "%s&deviceId=%s&clientType=%s&%s",
           SOCKET_PATH,
           DEVICE_ID,
           DEVICE_TYPE,
           authHeaders);

  // Setup secure connection
  if (USE_SSL) {
    webSocket.beginSSL(SERVER_HOST, SOCKET_PORT, fullPath, SSL_FINGERPRINT);
  } else {
    webSocket.begin(SERVER_HOST, SOCKET_PORT, fullPath);
  }
}

void socket_Connected(const char* payload, size_t length) {
  Serial.println("[WSc] Connected to server");
  connectionState = CONNECTED;
}

void socket_Disconnected(const char* payload, size_t length) {
  Serial.println("[WSc] Disconnected from server");
  connectionState = DISCONNECTED;
}

void socket_error(const char* payload, size_t length) {
  Serial.println("[WSc] Error: " + String(payload));
}

void socket_verifyCodeResult(const char* payload, size_t length) {
  Serial.println("[WSc] Received verifyCodeResult: " + String(payload));
  String message = String((char*)payload);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  bool success = doc["success"];
  if (success) {
    Serial.println("Code verified successfully");
    Serial.print("Box ");
    Serial.print(doc["doorId"].as<String>());
    Serial.println(" unlocked.");
    writeSerial2("verifyStatus;success");
    delay(2000);
    openDoor(doc["doorId"].as<String>());
  } else {
    Serial.println("Code verification failed. Wrong otp!");
    writeSerial2("verifyStatus;failed");
    ringWarning();
    failCount++;
  }
}

void socket_openDoor(const char* payload, size_t length) {
  Serial.println("[WSc] Received openDoor: " + String(payload));

  openDoor(String(payload));
  // After opening the door, send a success message
  JsonDocument doc;
  doc["success"] = true;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending openDoorResponse: " + output);
  webSocket.emit("openDoorResponse", output.c_str());
}

void sendVerifyCode(const char* otp) {
  if (connectionState != CONNECTED) {
    Serial.println("Locker not authenticated");
    return;
  }
  webSocket.emit("verifyCode", otp);
}

void sendBoxUsage(const String& doorId, bool isObject) {
  if (connectionState != CONNECTED) {
    Serial.println("Locker not authenticated");
    return;
  }

  JsonDocument doc;
  doc["doorId"] = doorId;
  doc["isObject"] = isObject;

  String output;
  serializeJson(doc, output);

  Serial.println("Sending boxUsage: " + output);
  webSocket.emit("boxUsage", output.c_str());
}

void sendWarning() {
  if (connectionState != CONNECTED) return;

  ringWarning();

  webSocket.emit("warning");
}
