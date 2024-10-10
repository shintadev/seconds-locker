#include "SerialCommunication.h"
#include "../Locker_Setup.h"
#include "WebSocketHandlers.h"
#include "LockerOperations.h"

void handleSerial2() {
  String* payload = readSerial2();

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
      bool doorState = checkDoorState(PCF8574_ADDRESS_1, i);
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

String* readSerial2() {
  static String array[10];
  int arrayIndex = 0;
  unsigned long lastDataReceived = millis();

  char buffer[128] = { 0 };  // Statical allocate buffer and initialize to zero

  while (Serial2.available() > 0 && (millis() - lastDataReceived) < 1000) {  // Increase timeout to 1000 ms
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
