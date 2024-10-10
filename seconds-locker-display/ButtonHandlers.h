#ifndef BUTTON_HANDLERS_H
#define BUTTON_HANDLERS_H

#include "../Locker_Setup.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <TFT_eWidget.h>

class ExtendedButtonWidget : public ButtonWidget {
public:
  ExtendedButtonWidget(TFT_eSPI *tft);
  void setPressAction(void (*pressAction)(void));
  void setReleaseAction(void (*releaseAction)(void));
  void pressAction();
  void releaseAction();

private:
  void (*_pressAction)(void) = nullptr;
  void (*_releaseAction)(void) = nullptr;
};

extern TFT_eSPI tft;
extern String userInput;
extern unsigned long inactivityTimer;
extern ExtendedButtonWidget *btnLanguageMenu[2];
extern ExtendedButtonWidget *btnVerifyMenu[2];
extern ExtendedButtonWidget *btnVerifyOTP[2];
extern ExtendedButtonWidget *btnVerifyQRCode[1];
extern ExtendedButtonWidget *btnAdminMenu[4];
extern ExtendedButtonWidget *btnOpenBoxAdminMenu[LOCKER_DOORS_NUM + 1];

void initializeButtons();
void handleButtonPresses(ExtendedButtonWidget *btn[], uint8_t btnNum, bool pressed, uint16_t t_x, uint16_t t_y);

// Declare all button action functions here
void btnLanguageMenu_EN_pressAction(void);
void btnLanguageMenu_EN_releaseAction(void);
void btnLanguageMenu_VN_pressAction(void);
void btnLanguageMenu_VN_releaseAction(void);
void btnVerifyMenu_OTP_pressAction(void);
void btnVerifyMenu_OTP_releaseAction(void);
void btnVerifyMenu_QRCode_pressAction(void);
void btnVerifyMenu_QRCode_releaseAction(void);
void btnVerifyOTP_Submit_pressAction(void);
void btnVerifyOTP_Submit_releaseAction(void);
void btnVerifyOTP_Cancel_pressAction(void);
void btnVerifyOTP_Cancel_releaseAction(void);
void btnVerifyQRCode_Cancel_pressAction(void);
void btnVerifyQRCode_Cancel_releaseAction(void);
void btnAdminMenu_CheckDoorsStatus_pressAction(void);
void btnAdminMenu_CheckDoorsStatus_releaseAction(void);
void btnAdminMenu_OpenBox_pressAction(void);
void btnAdminMenu_OpenBox_releaseAction(void);
void btnAdminMenu_Reset_pressAction(void);
void btnAdminMenu_Reset_releaseAction(void);
void btnAdminMenu_Exit_pressAction(void);
void btnAdminMenu_Exit_releaseAction(void);
void btnOpenBox_Back_pressAction(void);
void btnOpenBox_Back_releaseAction(void);
void btnOpenBox_Box_pressAction(void);
void btnOpenBox_Box_releaseAction(void);

#endif