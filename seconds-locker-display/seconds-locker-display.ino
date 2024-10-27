#define LOAD_GFXFF
#include "../config.h"
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

MakeFont myFont(&setPixel);

ExtendedButtonWidget *btnLanguageMenu[2];
ExtendedButtonWidget *btnVerifyMenu[2];
ExtendedButtonWidget *btnVerifyOTP[2];
ExtendedButtonWidget *btnVerifyQRCode[1];
ExtendedButtonWidget *btnAdminMenu[4];
ExtendedButtonWidget *btnOpenBoxAdminMenu[LOCKER_DOORS_NUM + 1];

ScreenState currentScreen = LANGUAGE_MENU;

String userInput = "";
String language = "EN";
uint32_t lastSerial2Read = 0;
uint32_t inactivityTimer = 0;
uint32_t serial2Timeout = 0;

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
  changeScreen(LOADING);
}

void loop() {
  uint32_t currentMillis = millis();

  handleCurrentScreen();

  if (currentMillis - serial2Timeout >= 10000) {
    Serial.println("Serial2 timeout");
    Serial.print("Time since last valid message: ");
    Serial.print((currentMillis - serial2Timeout) / 1000);
    Serial.println(" seconds");
    Serial.print("Current millis: ");
    Serial.println(currentMillis);
    Serial.print("Last serial2Timeout: ");
    Serial.println(serial2Timeout);
    changeScreen(LOADING);
    serial2Timeout = currentMillis;  // Reset the timeout to prevent continuous triggers
  }

  if (currentMillis - lastSerial2Read >= 500) {
    lastSerial2Read = currentMillis;
    handleSerial2();
  }

  tft.unloadFont();
  yield();
  delay(100);
}
