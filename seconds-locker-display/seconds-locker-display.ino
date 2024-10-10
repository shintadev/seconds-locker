#define LOAD_GFXFF
#include "../Locker_Setup.h"
#include "DisplaySetup.h"
#include "ButtonHandlers.h"
#include "ScreenManagement.h"
#include "KeypadHandler.h"
#include "SerialCommunication.h"
#include "FontMaker.h"
#include <TFT_eSPI.h>     // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library
#include <I2CKeyPad.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

MakeFont myfont(&setpx);

ExtendedButtonWidget *btnLanguageMenu[2];
ExtendedButtonWidget *btnVerifyMenu[2];
ExtendedButtonWidget *btnVerifyOTP[2];
ExtendedButtonWidget *btnVerifyQRCode[1];
ExtendedButtonWidget *btnAdminMenu[4];
ExtendedButtonWidget *btnOpenBoxAdminMenu[LOCKER_DOORS_NUM + 1];

ScreenState currentScreen = LANGUAGE_MENU;

String userInput = "";
unsigned long lastSerial2Read = 0;
unsigned long inactivityTimer = 0;
String language = "EN";

// Keypad setup
I2CKeyPad keyPad(KEYPAD_ADDRESS);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  initializeDisplay();

  // Initialize I2C communication
  Wire.begin();
  Wire.setClock(400000);

  // Initialize keypad
  setupKeypad();

  // Initialize button arrays
  initializeButtons();

  touch_calibrate();
  changeScreen(LANGUAGE_MENU);
}

void loop() {
  unsigned long currentMillis = millis();

  handleCurrentScreen();

  if (currentMillis - lastSerial2Read >= 500) {
    lastSerial2Read = currentMillis;
    handleSerial2();
  }

  tft.unloadFont();
  yield();
  delay(100);
}
