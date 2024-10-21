#include "../Locker_Setup.h"
#include "ButtonHandlers.h"
#include "ScreenManagement.h"
#include "SerialCommunication.h"

ExtendedButtonWidget::ExtendedButtonWidget(TFT_eSPI *tft)
  : ButtonWidget(tft) {}

void ExtendedButtonWidget::setPressAction(void (*pressAction)(void)) {
  _pressAction = pressAction;
}

void ExtendedButtonWidget::setReleaseAction(void (*releaseAction)(void)) {
  _releaseAction = releaseAction;
}

void ExtendedButtonWidget::pressAction() {
  if (_pressAction) _pressAction();
}

void ExtendedButtonWidget::releaseAction() {
  if (_releaseAction) _releaseAction();
}

void initializeButtons() {
  for (int i = 0; i < 2; i++) btnLanguageMenu[i] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 2; i++) btnVerifyMenu[i] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 2; i++) btnVerifyOTP[i] = new ExtendedButtonWidget(&tft);
  btnVerifyQRCode[0] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 4; i++) btnAdminMenu[i] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < LOCKER_DOORS_NUM + 1; i++) btnOpenBoxAdminMenu[i] = new ExtendedButtonWidget(&tft);
}

void handleButtonPresses(ExtendedButtonWidget *btn[], uint8_t btnNum, bool pressed, uint16_t t_x, uint16_t t_y) {
  for (uint8_t b = 0; b < btnNum; b++) {
    if (pressed) {
      inactivityTimer = millis();
      if (btn[b]->contains(t_x, t_y)) {
        if (!btn[b]->getState()) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
    } else {
      if (btn[b]->getState()) {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
}

void btnLanguageMenu_EN_pressAction(void) {
  if (btnLanguageMenu[0]->justPressed()) {
    btnLanguageMenu[0]->drawSmoothButton(true);
    Serial.print("Button Language Menu EN is touched");
  }
}

void btnLanguageMenu_EN_releaseAction(void) {
  if (btnLanguageMenu[0]->justReleased() && currentScreen == LANGUAGE_MENU) {
    btnLanguageMenu[0]->drawSmoothButton(false);
    Serial.println("Button Language Menu EN is released");
    language = "EN";
    changeScreen(VERIFY_MENU);
  }
}

void btnLanguageMenu_VN_pressAction(void) {
  if (btnLanguageMenu[1]->justPressed()) {
    btnLanguageMenu[1]->drawSmoothButton(true);
    Serial.print("Button Language Menu VN is touched");
  }
}

void btnLanguageMenu_VN_releaseAction(void) {
  if (btnLanguageMenu[1]->justReleased() && currentScreen == LANGUAGE_MENU) {
    btnLanguageMenu[1]->drawSmoothButton(false);
    Serial.println("Button Language Menu VN is released");
    language = "VN";
    changeScreen(VERIFY_MENU);
  }
}

void btnVerifyMenu_OTP_pressAction(void) {
  if (btnVerifyMenu[0]->justPressed()) {
    btnVerifyMenu[0]->drawSmoothButton(true);
    Serial.print("Button Verify Menu OTP is touched");
  }
}

void btnVerifyMenu_OTP_releaseAction(void) {
  if (btnVerifyMenu[0]->justReleased() && currentScreen == VERIFY_MENU) {
    btnVerifyMenu[0]->drawSmoothButton(false);
    Serial.println("Button Verify Menu OTP is released");
    changeScreen(VERIFY_OTP);
  }
}

void btnVerifyMenu_QRCode_pressAction(void) {
  if (btnVerifyMenu[1]->justPressed()) {
    btnVerifyMenu[1]->drawSmoothButton(true);
    Serial.print("Button Verify Menu QRCode is touched");
  }
}

void btnVerifyMenu_QRCode_releaseAction(void) {
  if (btnVerifyMenu[1]->justReleased() && currentScreen == VERIFY_MENU) {
    btnVerifyMenu[1]->drawSmoothButton(false);
    Serial.println("Button Verify Menu QRCode is released");
    writeSerial2("sendQR");
    changeScreen(VERIFY_QRCODE);
  }
}

void btnVerifyOTP_Submit_pressAction(void) {
  if (btnVerifyOTP[0]->justPressed()) {
    btnVerifyOTP[0]->drawSmoothButton(true);
    Serial.print("Button Verify OTP Submit is touched");
  }
}

void btnVerifyOTP_Submit_releaseAction(void) {
  if (btnVerifyOTP[0]->justReleased() && currentScreen == VERIFY_OTP) {
    btnVerifyOTP[0]->drawSmoothButton(false);
    Serial.println("Button Verify OTP Submit is released\n" + userInput);
    if (userInput.length() == 6) {
      writeSerial2("verifyCode;" + userInput);
      userInput = "";
      changeScreen(MESSAGE);
      displayMessage("Verifying OTP...");
    } else {
      displayMessage("OTP must be 6 digits");
      delay(2000);
      changeScreen(VERIFY_OTP);
    }
  }
}

void btnVerifyOTP_Cancel_pressAction(void) {
  if (btnVerifyOTP[1]->justPressed()) {
    btnVerifyOTP[1]->drawSmoothButton(true);
    Serial.println("Button Verify OTP Cancel is touched");
  }
}

void btnVerifyOTP_Cancel_releaseAction(void) {
  if (btnVerifyOTP[1]->justReleased() && currentScreen == VERIFY_OTP) {
    btnVerifyOTP[1]->drawSmoothButton(false);
    Serial.println("Button Verify OTP Cancel is released");
    userInput = "";
    changeScreen(MESSAGE);
    displayMessage("Cancelling OTP verification...");
    delay(2000);
    changeScreen(LANGUAGE_MENU);
  }
}

void btnVerifyQRCode_Cancel_pressAction(void) {
  if (btnVerifyQRCode[0]->justPressed()) {
    btnVerifyQRCode[0]->drawSmoothButton(true);
    Serial.println("Button Verify QRCode Cancel is touched");
  }
}

void btnVerifyQRCode_Cancel_releaseAction(void) {
  if (btnVerifyQRCode[0]->justReleased() && currentScreen == VERIFY_QRCODE) {
    btnVerifyQRCode[0]->drawSmoothButton(false);
    Serial.println("Button Verify QRCode Cancel is released");
    changeScreen(MESSAGE);
    displayMessage("Cancelling QR Code verification...");
    delay(2000);
    changeScreen(LANGUAGE_MENU);
  }
}

void btnAdminMenu_CheckDoorsStatus_pressAction(void) {
  if (btnAdminMenu[0]->justPressed()) {
    btnAdminMenu[0]->drawSmoothButton(true);
    Serial.println("Button Admin Menu Check Doors Status is touched");
  }
}

void btnAdminMenu_CheckDoorsStatus_releaseAction(void) {
  if (btnAdminMenu[0]->justReleased() && currentScreen == ADMIN_MENU) {
    btnAdminMenu[0]->drawSmoothButton(false);
    Serial.println("Button Admin Menu Check Doors Status is released");
    writeSerial2("checkStatus");
    changeScreen(MESSAGE);
    displayMessage("Checking doors status...");
  }
}

void btnAdminMenu_OpenBox_pressAction(void) {
  if (btnAdminMenu[1]->justPressed()) {
    btnAdminMenu[1]->drawSmoothButton(true);
    Serial.println("Button Admin Menu Open Box is touched");
  }
}

void btnAdminMenu_OpenBox_releaseAction(void) {
  if (btnAdminMenu[1]->justReleased() && currentScreen == ADMIN_MENU) {
    btnAdminMenu[1]->drawSmoothButton(false);
    Serial.println("Button Admin Menu Open Box is released");
    changeScreen(OPEN_BOX_ADMIN_MENU);
  }
}

void btnAdminMenu_Reset_pressAction(void) {
  if (btnAdminMenu[2]->justPressed()) {
    btnAdminMenu[2]->drawSmoothButton(true);
    Serial.println("Button Admin Menu Reset is touched");
  }
}

void btnAdminMenu_Reset_releaseAction(void) {
  if (btnAdminMenu[2]->justReleased() && currentScreen == ADMIN_MENU) {
    btnAdminMenu[2]->drawSmoothButton(false);
    Serial.println("Button Admin Menu Reset is released");
    writeSerial2("reset");
    changeScreen(MESSAGE);
    for (int i = 0; i < 5; i++) {
      displayMessage("Resetting.");
      delay(1000);
      displayMessage("Resetting.");
      delay(1000);
      displayMessage("Resetting...");
      delay(1000);
    }

    ESP.restart();
  }
}

void btnAdminMenu_Exit_pressAction(void) {
  if (btnAdminMenu[3]->justPressed()) {
    btnAdminMenu[3]->drawSmoothButton(true);
    Serial.println("Button Admin Menu Exit is touched");
  }
}

void btnAdminMenu_Exit_releaseAction(void) {
  if (btnAdminMenu[3]->justReleased() && currentScreen == ADMIN_MENU) {
    btnAdminMenu[3]->drawSmoothButton(false);
    Serial.println("Button Admin Menu Exit is released");
    changeScreen(MESSAGE);
    displayMessage("Exiting Admin Mode...");
    delay(2000);
    changeScreen(LANGUAGE_MENU);
  }
}

void btnOpenBox_Back_pressAction(void) {
  if (btnOpenBoxAdminMenu[0]->justPressed()) {
    btnOpenBoxAdminMenu[0]->drawSmoothButton(true);
    Serial.println("Button Open Box Back is touched");
  }
}

void btnOpenBox_Back_releaseAction(void) {
  if (btnOpenBoxAdminMenu[0]->justReleased() && currentScreen == OPEN_BOX_ADMIN_MENU) {
    btnOpenBoxAdminMenu[0]->drawSmoothButton(false);
    Serial.println("Button Open Box Back is released");
    changeScreen(ADMIN_MENU);
  }
}

void btnOpenBox_Box_pressAction(void) {
  for (int i = 1; i <= LOCKER_DOORS_NUM; i++) {
    if (btnOpenBoxAdminMenu[i]->justPressed()) {
      btnOpenBoxAdminMenu[i]->drawSmoothButton(true);
      Serial.printf("Button Open Box %d is touched\n", i);
    }
  }
}

void btnOpenBox_Box_releaseAction(void) {
  for (int i = 1; i <= LOCKER_DOORS_NUM; i++) {
    if (btnOpenBoxAdminMenu[i]->justReleased() && currentScreen == OPEN_BOX_ADMIN_MENU) {
      btnOpenBoxAdminMenu[i]->drawSmoothButton(false);
      Serial.printf("Button Open Box %d is released\n", i);
      writeSerial2("openBoxAdmin;" + String(i));
    }
  }
}