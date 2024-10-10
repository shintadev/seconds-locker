#include "LockerOperations.h"
#include "../Locker_Setup.h"
#include "WebSocketHandlers.h"
#include "SerialCommunication.h"
#include "RFIDHandler.h"
#include <Wire.h>

void handleLockerOperations() {
  if (rfid.PICC_IsNewCardPresent()) {
    if (readRFID()) {
      writeSerial2("ADMIN");
    }  // Allow other processes to run
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
    Serial.println("Checking door state");
    int doorState = checkDoorState(PCF8574_ADDRESS_1, doorIndex);
    Serial.println(doorState);
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
      int doorState = checkDoorState(PCF8574_ADDRESS_1, doorIndex);
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
    while (checkDoorState(PCF8574_ADDRESS_1, doorIndex) == 0) {  // Assuming 0 means door is open
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

void openDoorAdmin(const int doorIndex) {
  Serial.printf("Unlocking door: %d\n", doorIndex);
  unlockBox(pwm, doorIndex);
  delay(1000);
  Serial.printf("Unlocked door: %d\n", doorIndex);

  bool doorClosed = true;

  // Wait for the door to be opened
  while (doorClosed) {
    int doorState = checkDoorState(PCF8574_ADDRESS_1, doorIndex);
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
    int doorState = checkDoorState(PCF8574_ADDRESS_1, doorIndex);
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

bool checkDoorState(int pcf8574Addr, int pcfPin) {
  Serial.print(Wire.available());
  Serial.print(" ");
  Serial.println(Wire.requestFrom(pcf8574Addr, 1));
  if (Wire.requestFrom(pcf8574Addr, 1) && Wire.available()) {
    uint8_t state = Wire.read();
    bool pinState = !(state & (1 << pcfPin));

    Serial.print("Pin State: ");
    Serial.println(pinState);
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

void unlockBox(Adafruit_PWMServoDriver& pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(105));  // Unlock position
}

void lockBox(Adafruit_PWMServoDriver& pwm, int servoChannel) {
  pwm.setPWM(servoChannel, 0, angleToPulse(20));  // Lock position
}

int angleToPulse(int angle) {
  int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  return pulse;
}
