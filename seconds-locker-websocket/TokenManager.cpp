#include "TokenManager.h"
#include "../Locker_Setup.h"
#include <EEPROM.h>

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
