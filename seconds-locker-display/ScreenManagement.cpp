#include "../Locker_Setup.h"
#include "ScreenManagement.h"
#include "ButtonHandlers.h"
#include "KeypadHandler.h"
#include <qrcode_generator.h>

#define TITLE 0x31a6
#define BACKGROUND 0xf7be
#define BUTTON_FILL 0x1232
#define BUTTON_FILL_2 0x029d
#define BUTTON_TEXT 0xffff
#define BUTTON_BORDER 0x55ff
#define TEXTFIELD_FILL 0xffff
#define TEXTFIELD_BORDER 0xc638
#define TEXTFIELD_TEXT 0x0000

QRCode qrcode;

void changeScreen(ScreenState newScreen) {
  currentScreen = newScreen;
  tft.fillScreen(BACKGROUND);

  // Clear all buttons
  for (int i = 0; i < 2; i++) btnLanguageMenu[i]->press(false);
  for (int i = 0; i < 2; i++) btnVerifyMenu[i]->press(false);
  for (int i = 0; i < 2; i++) btnVerifyOTP[i]->press(false);
  btnVerifyQRCode[0]->press(false);
  for (int i = 0; i < 4; i++) btnAdminMenu[i]->press(false);
  for (int i = 0; i < LOCKER_DOORS_NUM + 1; i++) btnOpenBoxAdminMenu[i]->press(false);

  switch (newScreen) {
    case LANGUAGE_MENU:
      setupLanguageMenu();
      break;
    case VERIFY_MENU:
      setupVerifyMenu();
      break;
    case VERIFY_OTP:
      setupVerifyOTP();
      break;
    case VERIFY_QRCODE:
      setupVerifyQRCode();
      break;
    case ADMIN_MENU:
      setupAdminMenu();
      break;
    case OPEN_BOX_ADMIN_MENU:
      setupOpenBoxAdminMenu();
      break;
    case MESSAGE:
      // No setup needed for message screen
      break;
  }
}

void handleCurrentScreen() {
  switch (currentScreen) {
    case LANGUAGE_MENU:
      handleLanguageMenu();
      break;
    case VERIFY_MENU:
      handleVerifyMenu();
      break;
    case VERIFY_OTP:
      char buffer[7];
      handleVerifyOTP(buffer, sizeof(buffer), 20000);
      break;
    case VERIFY_QRCODE:
      handleVerifyQRCode();
      break;
    case ADMIN_MENU:
      handleAdminMenu();
      break;
    case OPEN_BOX_ADMIN_MENU:
      handleOpenBoxAdminMenu();
      break;
    case MESSAGE:
      handleMessage();
      break;
  }
}

void setupLanguageMenu() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2;
  // tft.setTextColor(TFT_BLACK);
  // tft.setCursor(50, 50);
  // tft.drawString("Choose a language",50,50);
  myFont.print(100, 50, const_cast<char*>("Choose a language:"), TITLE, BACKGROUND);

  btnLanguageMenu[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("EN"), 1);
  btnLanguageMenu[0]->setPressAction(btnLanguageMenu_EN_pressAction);
  btnLanguageMenu[0]->setReleaseAction(btnLanguageMenu_EN_releaseAction);
  btnLanguageMenu[0]->drawSmoothButton(false, 3, BACKGROUND);

  y = tft.height() / 2 + BUTTON_H + 10;
  btnLanguageMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("VN"), 1);
  btnLanguageMenu[1]->setPressAction(btnLanguageMenu_VN_pressAction);
  btnLanguageMenu[1]->setReleaseAction(btnLanguageMenu_VN_releaseAction);
  btnLanguageMenu[1]->drawSmoothButton(false, 3, BACKGROUND);
}

void setupVerifyMenu() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2;
  // tft.setTextColor(TFT_BLACK);
  // tft.setCursor(50, 50);

  if (language == "EN") {
    myFont.print(50, 50, const_cast<char*>("Select a verification method:"), TITLE, BACKGROUND);
  } else if (language == "VN") {
    myFont.print(50, 50, const_cast<char*>("Chọn phương thức xác thực:"), TITLE, BACKGROUND);
  }

  btnVerifyMenu[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("OTP"), 1);
  btnVerifyMenu[0]->setPressAction(btnVerifyMenu_OTP_pressAction);
  btnVerifyMenu[0]->setReleaseAction(btnVerifyMenu_OTP_releaseAction);
  btnVerifyMenu[0]->drawSmoothButton(false, 3, BACKGROUND);

  y = tft.height() / 2 + BUTTON_H + 10;
  btnVerifyMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("QR Code"), 1);
  btnVerifyMenu[1]->setPressAction(btnVerifyMenu_QRCode_pressAction);
  btnVerifyMenu[1]->setReleaseAction(btnVerifyMenu_QRCode_releaseAction);
  btnVerifyMenu[1]->drawSmoothButton(false, 3, BACKGROUND);
}

void setupVerifyOTP() {
  uint16_t x = (tft.width() - BUTTON_W) / 4;
  uint16_t y = tft.height() * 3 / 4;
  tft.setTextColor(TFT_BLACK);
  // tft.setCursor(50, 50);

  if (language == "EN") {
    myFont.print(100, 50, const_cast<char*>("Enter OTP"), TITLE, BACKGROUND);
  } else if (language == "VN") {
    myFont.print(100, 50, const_cast<char*>("Nhập mã OTP"), TITLE, BACKGROUND);
  }

  // Calculate the position to center the text field
  uint16_t fieldWidth = tft.width() / 2;
  uint16_t fieldHeight = 50;

  tft.fillRoundRect((tft.width() - fieldWidth) / 2, (tft.height() - fieldHeight) / 2, fieldWidth, 50, 5, TEXTFIELD_FILL);    // Draw a text field
  tft.drawRoundRect((tft.width() - fieldWidth) / 2, (tft.height() - fieldHeight) / 2, fieldWidth, 50, 5, TEXTFIELD_BORDER);  // Add a border

  tft.setTextDatum(MC_DATUM);
  tft.drawString(userInput, tft.width() / 2, tft.height() / 2, 4);  // Center text inside the box

  btnVerifyOTP[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("Submit"), 1);
  btnVerifyOTP[0]->setPressAction(btnVerifyOTP_Submit_pressAction);
  btnVerifyOTP[0]->setReleaseAction(btnVerifyOTP_Submit_releaseAction);
  btnVerifyOTP[0]->drawSmoothButton(false, 3, BACKGROUND);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnVerifyOTP[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL_2, BUTTON_TEXT, const_cast<char*>("Cancel"), 1);
  btnVerifyOTP[1]->setPressAction(btnVerifyOTP_Cancel_pressAction);
  btnVerifyOTP[1]->setReleaseAction(btnVerifyOTP_Cancel_releaseAction);
  btnVerifyOTP[1]->drawSmoothButton(false, 3, BACKGROUND);
}

void setupVerifyQRCode() {
  uint16_t x = tft.width() - 80;
  uint16_t y = 30;

  // QR code parameters
  int qrElementSize = 3;  // Increase the size of each QR code element
  int qrVersion = 14;
  int qrSize = (4 * qrVersion + 17) * qrElementSize;
  int qrX = (tft.width() - qrSize) / 2;   // Centered horizontally
  int qrY = (tft.height() - qrSize) / 2;  // Positioned vertically

  // Draw a border around the QR code
  int borderWidth = 10;  // Width of the border in pixels
  int totalSizeWithBorder = qrSize + (2 * borderWidth);

  // Draw border
  tft.fillRect(qrX - borderWidth, qrY - borderWidth, totalSizeWithBorder, totalSizeWithBorder, TFT_WHITE);

  //Display QR Code
  displayQRcode(qrX, qrY, qrElementSize, qrVersion, ECC_HIGH, "This function is unavailable now");

  btnVerifyQRCode[0]->initButtonUL(x, y, BUTTON_W - 100, BUTTON_H, BUTTON_BORDER, BUTTON_FILL_2, BUTTON_TEXT, const_cast<char*>("X"), 1);
  btnVerifyQRCode[0]->setPressAction(btnVerifyQRCode_Cancel_pressAction);
  btnVerifyQRCode[0]->setReleaseAction(btnVerifyQRCode_Cancel_releaseAction);
  btnVerifyQRCode[0]->drawSmoothButton(false, 3, BACKGROUND);
}

void setupAdminMenu() {
  uint16_t x = (tft.width() - BUTTON_W) / 4;
  uint16_t y = tft.height() / 2;
  tft.setTextColor(TITLE);
  tft.setCursor(50, 50);
  tft.println("Welcome to Admin Mode");

  btnAdminMenu[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("Status"), 1);
  btnAdminMenu[0]->setPressAction(btnAdminMenu_CheckDoorsStatus_pressAction);
  btnAdminMenu[0]->setReleaseAction(btnAdminMenu_CheckDoorsStatus_releaseAction);
  btnAdminMenu[0]->drawSmoothButton(false, 3, BACKGROUND);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnAdminMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("Open Box"), 1);
  btnAdminMenu[1]->setPressAction(btnAdminMenu_OpenBox_pressAction);
  btnAdminMenu[1]->setReleaseAction(btnAdminMenu_OpenBox_releaseAction);
  btnAdminMenu[1]->drawSmoothButton(false, 3, BACKGROUND);

  x = (tft.width() - BUTTON_W) / 4;
  y = tft.height() / 2 + BUTTON_H + 10;
  btnAdminMenu[2]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("Reset"), 1);
  btnAdminMenu[2]->setPressAction(btnAdminMenu_Reset_pressAction);
  btnAdminMenu[2]->setReleaseAction(btnAdminMenu_Reset_releaseAction);
  btnAdminMenu[2]->drawSmoothButton(false, 3, BACKGROUND);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnAdminMenu[3]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("Exit"), 1);
  btnAdminMenu[3]->setPressAction(btnAdminMenu_Exit_pressAction);
  btnAdminMenu[3]->setReleaseAction(btnAdminMenu_Exit_releaseAction);
  btnAdminMenu[3]->drawSmoothButton(false, 3, BACKGROUND);
}

void setupOpenBoxAdminMenu() {
  uint16_t x = tft.width() * 3 / 4;
  uint16_t y = 50;
  tft.setTextColor(TITLE);
  tft.setCursor(50, 50);
  tft.println("Open Box");

  btnOpenBoxAdminMenu[0]->initButtonUL(x, y, 100, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>("Back"), 1);
  btnOpenBoxAdminMenu[0]->setPressAction(btnOpenBox_Back_pressAction);
  btnOpenBoxAdminMenu[0]->setReleaseAction(btnOpenBox_Back_releaseAction);
  btnOpenBoxAdminMenu[0]->drawSmoothButton(false, 3, BACKGROUND);

  x = (tft.width() - BUTTON_W) / 4;
  y = tft.height() / 2;
  for (int i = 1; i <= LOCKER_DOORS_NUM; i++) {
    x = (tft.width() - BUTTON_W) * (i % 2 == 1 ? 1 / 4 : 3 / 4);
    y = tft.height() / 2 + (i % 2 == 0 ? BUTTON_H + 10 : 0);

    char buttonLabel[10];
    snprintf(buttonLabel, sizeof(buttonLabel), "Box %d", i);

    btnOpenBoxAdminMenu[i]->initButtonUL(x, y, BUTTON_W, BUTTON_H, BUTTON_BORDER, BUTTON_FILL, BUTTON_TEXT, const_cast<char*>(buttonLabel), 1);
    btnOpenBoxAdminMenu[i]->setPressAction(btnOpenBox_Box_pressAction);
    btnOpenBoxAdminMenu[i]->setReleaseAction(btnOpenBox_Box_releaseAction);
    btnOpenBoxAdminMenu[i]->drawSmoothButton(false, 3, BACKGROUND);
  }
}

void displayMessage(String message) {
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TITLE);
  tft.setCursor(50, tft.height() / 2 - 50);
  tft.println(message);
}

void handleLanguageMenu() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();

    handleButtonPresses(btnLanguageMenu, 2, pressed, t_x, t_y);

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }
  }
}

void handleVerifyMenu() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();

    handleButtonPresses(btnVerifyMenu, 2, pressed, t_x, t_y);

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    if (millis() - inactivityTimer > 30000) {
      changeScreen(LANGUAGE_MENU);
    }
  }
}

void handleVerifyOTP(char* buffer, uint8_t length, uint16_t timeout) {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates
  uint32_t start = millis();
  const uint32_t debounceDelay = 200;
  uint8_t bufferIndex = 0;
  uint32_t lastKeyPressTime = 0;

  buffer[0] = '\0';

  while (millis() - start < timeout && currentScreen == VERIFY_OTP) {
    // Scan keys every 50ms at most
    if (millis() - scanTime >= 50) {
      // Pressed will be set true if there is a valid touch on the screen
      bool pressed = tft.getTouch(&t_x, &t_y);
      scanTime = millis();

      handleButtonPresses(btnVerifyOTP, 2, pressed, t_x, t_y);

      // Debugging: Print touch coordinates to serial monitor
      // if (pressed) {
      //   Serial.print("Touch at: ");
      //   Serial.print(t_x);
      //   Serial.print(", ");
      //   Serial.println(t_y);
      // }
    }

    handleKeypadInput(lastKeyPressTime, debounceDelay, buffer, bufferIndex, length);
  }

  if (millis() - start >= timeout) {
    Serial.println("Time out");
    btnVerifyOTP[1]->drawSmoothButton(false);
    changeScreen(MESSAGE);
    displayMessage("Cancelling OTP verification...");
    userInput = "";
    delay(2000);
    changeScreen(LANGUAGE_MENU);
  }
}

void handleVerifyQRCode() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();

    handleButtonPresses(btnVerifyQRCode, 1, pressed, t_x, t_y);

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    if (millis() - inactivityTimer > 30000) {
      changeScreen(LANGUAGE_MENU);
    }
  }
}

void handleAdminMenu() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();

    handleButtonPresses(btnAdminMenu, 4, pressed, t_x, t_y);

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }
  }
}

void handleOpenBoxAdminMenu() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();

    handleButtonPresses(btnOpenBoxAdminMenu, LOCKER_DOORS_NUM + 1, pressed, t_x, t_y);

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }
  }
}

void handleMessage() {
  // Handle message screen updates if needed
}

void displayQRcode(int offset_x, int offset_y, int element_size, int QRsize, int ECC_Mode, const char* Message) {
  Serial.println("Starting QR code generation");

  if (QRsize < 1 || QRsize > 40) {
    Serial.println("Invalid QR size");
    return;
  }

  if (ECC_Mode < 0 || ECC_Mode > 3) {
    Serial.println("Invalid ECC mode");
    return;
  }

  int bufferSize = qrcode_getBufferSize(QRsize);
  uint8_t qrcodeData[bufferSize];

  if (ECC_Mode == 0) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_LOW, Message);
  if (ECC_Mode == 1) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_MEDIUM, Message);
  if (ECC_Mode == 2) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_QUARTILE, Message);
  if (ECC_Mode == 3) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_HIGH, Message);

  for (int y = 0; y < qrcode.size; y++) {
    for (int x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(x * element_size + offset_x, y * element_size + offset_y, element_size, element_size, TFT_BLACK);
      } else {
        tft.fillRect(x * element_size + offset_x, y * element_size + offset_y, element_size, element_size, TFT_WHITE);
      }
    }
  }
  Serial.println("Finished QR code generation");
}