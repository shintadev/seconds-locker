#ifndef SCREEN_MANAGEMENT_H
#define SCREEN_MANAGEMENT_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <I2CKeyPad.h>
#include "ButtonHandlers.h"
#include "FontMaker.h"

enum ScreenState {
  LANGUAGE_MENU,
  VERIFY_MENU,
  VERIFY_OTP,
  VERIFY_QRCODE,
  ADMIN_MENU,
  OPEN_BOX_ADMIN_MENU,
  MESSAGE
};

extern TFT_eSPI tft;
extern MakeFont myfont;
extern ScreenState currentScreen;
extern String userInput;
extern unsigned long inactivityTimer;
extern String language;
extern I2CKeyPad keyPad;
extern ExtendedButtonWidget *btnLanguageMenu[2];
extern ExtendedButtonWidget *btnVerifyMenu[2];
extern ExtendedButtonWidget *btnVerifyOTP[2];
extern ExtendedButtonWidget *btnVerifyQRCode[1];
extern ExtendedButtonWidget *btnAdminMenu[4];
extern ExtendedButtonWidget *btnOpenBoxAdminMenu[LOCKER_DOORS_NUM + 1];

void changeScreen(ScreenState newScreen);
void handleCurrentScreen();
void setupLanguageMenu();
void setupVerifyMenu();
void setupVerifyOTP();
void setupVerifyQRCode();
void setupAdminMenu();
void setupOpenBoxAdminMenu();
void displayMessage(String message);
void handleLanguageMenu();
void handleVerifyMenu();
void handleVerifyOTP(char *buffer, uint8_t length, uint16_t timeout);
void handleVerifyQRCode();
void handleAdminMenu();
void handleOpenBoxAdminMenu();
void handleMessage();
void displayQRcode(int offset_x, int offset_y, int element_size, int QRsize, int ECC_Mode, const char *Message);

#endif