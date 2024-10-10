#ifndef KEYPAD_HANDLER_H
#define KEYPAD_HANDLER_H

#include <I2CKeyPad.h>

extern I2CKeyPad keyPad;
extern String userInput;
extern unsigned long inactivityTimer;

void setupKeypad();
void handleKeypadInput(uint32_t &lastKeyPressTime, uint32_t debounceDelay, char *&buffer, uint8_t &bufferIndex, uint8_t &length);

#endif