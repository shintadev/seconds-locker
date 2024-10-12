#include "KeypadHandler.h"
#include "ScreenManagement.h"

char keymap[19] = "123A456B789C*0#DNF";  // ... NoKey  Fail }

void setupKeypad() {
  while (!keyPad.begin()) {
    displayMessage("ERROR! Please reboot.");
    Serial.println("ERROR: cannot communicate to keypad.\nPlease reboot.");
    delay(1000);
    ESP.restart();
  }

  keyPad.loadKeyMap(keymap);
  Serial.println("Keypad connected.");
}

void handleKeypadInput(uint32_t &lastKeyPressTime, uint32_t debounceDelay, char *&buffer, uint8_t &bufferIndex, uint8_t &length) {
  char key = keyPad.getChar();

  if (key != 'N' && key != 'F' && millis() - lastKeyPressTime > debounceDelay) {
    lastKeyPressTime = millis();
    inactivityTimer = millis();
    if (key == '*') {
      bufferIndex = 0;
      buffer[0] = '\0';
      userInput = "";
    } else if (bufferIndex < length - 1) {
      buffer[bufferIndex++] = key;
      buffer[bufferIndex] = '\0';
      userInput = buffer;
    }

    setupVerifyOTP();
  }
}