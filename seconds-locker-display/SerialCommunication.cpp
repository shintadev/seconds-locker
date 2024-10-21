#include "SerialCommunication.h"
#include "../Locker_Setup.h"
#include "ScreenManagement.h"

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

  if (payload[0] == "ADMIN") {
    Serial.println("Enter Admin Mode");
    changeScreen(ADMIN_MENU);
  } else if (payload[0] == "status") {
    for (int i = 0; i < sizeof(payload); i++) {
      displayMessage(payload[i + 1]);
      delay(1500);
    }
    changeScreen(ADMIN_MENU);
  } else if (payload[0] == "QR") {
    // show QR code
  } else if (payload[0] == "verifyStatus") {
    if (payload[1] == "success") {
      changeScreen(MESSAGE);
      displayMessage("Verified");
    } else if (payload[1] == "failed") {
      displayMessage("Verification failed");
      delay(2000);
      changeScreen(LANGUAGE_MENU);
    }
  } else if (payload[0] == "warning") {
    displayMessage("Time is up, please close the door");
  } else if (payload[0] == "offWarning") {
    displayMessage("Door is closed");
    delay(1000);
    changeScreen(LANGUAGE_MENU);
  } else if (payload[0] == "openDoor") {
    changeScreen(MESSAGE);
    displayMessage("Door " + payload[1] + " is opening");
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
