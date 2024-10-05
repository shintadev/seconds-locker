#define LOAD_GFXFF
#include "../Locker_Setup.h"
#include <FS.h>
#include "Free_Fonts.h"   // Include the header file attached to this sketch
#include <TFT_eSPI.h>     // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library
#include "FontMaker.h"
#include <I2CKeyPad.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// #define CALIBRATION_FILE "/TouchCalData1"
// #define REPEAT_CAL false

void setpx(int16_t x, int16_t y, uint16_t color) {
  tft.drawPixel(x, y, color);  //Thay đổi hàm này thành hàm vẽ pixel mà thư viện led bạn dùng cung cấp
}

MakeFont myfont(&setpx);

class ExtendedButtonWidget : public ButtonWidget {
public:
  ExtendedButtonWidget(TFT_eSPI *tft)
    : ButtonWidget(tft) {}

  void setPressAction(void (*pressAction)(void)) {
    _pressAction = pressAction;
  }

  void setReleaseAction(void (*releaseAction)(void)) {
    _releaseAction = releaseAction;
  }

  void pressAction() {
    if (_pressAction) _pressAction();
  }

  void releaseAction() {
    if (_releaseAction) _releaseAction();
  }

private:
  void (*_pressAction)(void) = nullptr;
  void (*_releaseAction)(void) = nullptr;
};

ExtendedButtonWidget *btnLanguageMenu[2];
ExtendedButtonWidget *btnVerifyMenu[2];
ExtendedButtonWidget *btnVerifyOTP[2];
ExtendedButtonWidget *btnVerifyQRCode[1];
ExtendedButtonWidget *btnAdminMenu[4];
ExtendedButtonWidget *btnOpenBoxAdminMenu[5];

#define BUTTON_W 150
#define BUTTON_H 50

enum ScreenState {
  LANGUAGE_MENU,
  VERIFY_MENU,
  VERIFY_OTP,
  VERIFY_QRCODE,
  ADMIN_MENU,
  OPEN_BOX_ADMIN_MENU,
  MESSAGE
};

ScreenState currentScreen = LANGUAGE_MENU;

String userInput = "";
unsigned long lastSerial2Read = 0;

void setupLanguageMenu();
void setupVerifyMenu();
void setupVerifyOTP();
void setupVerifyQRCode();
void setupAdminMenu();
void setupOpenBoxAdminMenu();
void displayMessage(String message);

void changeScreen(ScreenState newScreen);

String language = "EN";

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
    writeSerial2("verifyCode;" + userInput);
    userInput = "";
    changeScreen(MESSAGE);
    displayMessage("Verifying OTP...");
    delay(2000);
    // TODO: Add action for this button
    // verify OTP
    // if OTP is correct, open the box
    // if OTP is incorrect, display "OTP is incorrect"
    // back to language menu screen
    changeScreen(LANGUAGE_MENU);
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

void btnOpenBox_Box1_pressAction(void) {
  if (btnOpenBoxAdminMenu[1]->justPressed()) {
    btnOpenBoxAdminMenu[1]->drawSmoothButton(true);
    Serial.println("Button Open Box Box1 is touched");
  }
}

void btnOpenBox_Box1_releaseAction(void) {
  if (btnOpenBoxAdminMenu[1]->justReleased() && currentScreen == OPEN_BOX_ADMIN_MENU) {
    btnOpenBoxAdminMenu[1]->drawSmoothButton(false);
    Serial.println("Button Open Box Box1 is released");
    writeSerial2("openBoxAdmin;1");
  }
}

void btnOpenBox_Box2_pressAction(void) {
  if (btnOpenBoxAdminMenu[2]->justPressed()) {
    btnOpenBoxAdminMenu[2]->drawSmoothButton(true);
    Serial.println("Button Open Box Box2 is touched");
  }
}

void btnOpenBox_Box2_releaseAction(void) {
  if (btnOpenBoxAdminMenu[2]->justReleased() && currentScreen == OPEN_BOX_ADMIN_MENU) {
    btnOpenBoxAdminMenu[2]->drawSmoothButton(false);
    Serial.println("Button Open Box Box2 is released");
    writeSerial2("openBoxAdmin;2");
  }
}

void btnOpenBox_Box3_pressAction(void) {
  if (btnOpenBoxAdminMenu[3]->justPressed()) {
    btnOpenBoxAdminMenu[3]->drawSmoothButton(true);
    Serial.println("Button Open Box Box3 is touched");
  }
}

void btnOpenBox_Box3_releaseAction(void) {
  if (btnOpenBoxAdminMenu[3]->justReleased() && currentScreen == OPEN_BOX_ADMIN_MENU) {
    btnOpenBoxAdminMenu[3]->drawSmoothButton(false);
    Serial.println("Button Open Box Box3 is released");
    writeSerial2("openBoxAdmin;3");
  }
}

void btnOpenBox_Box4_pressAction(void) {
  if (btnOpenBoxAdminMenu[4]->justPressed()) {
    btnOpenBoxAdminMenu[4]->drawSmoothButton(true);
    Serial.println("Button Open Box Box4 is touched");
  }
}

void btnOpenBox_Box4_releaseAction(void) {
  if (btnOpenBoxAdminMenu[4]->justReleased() && currentScreen == OPEN_BOX_ADMIN_MENU) {
    btnOpenBoxAdminMenu[4]->drawSmoothButton(false);
    Serial.println("Button Open Box Box4 is released");
    writeSerial2("openBoxAdmin;4");
  }
}

// Keypad setup
#define KEYPAD_ADDRESS 0x20
I2CKeyPad keyPad(KEYPAD_ADDRESS);

char keymap[19] = "123A456B789C*0#DNF";  // ... NoKey  Fail }

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF18);

  // Initialize I2C communication
  Wire.begin();
  Wire.setClock(400000);

  // Initialize keypad
  setupKeypad();

  myfont.set_font(SansSerif16);

  // Initialize button arrays
  for (int i = 0; i < 2; i++) btnLanguageMenu[i] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 2; i++) btnVerifyMenu[i] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 2; i++) btnVerifyOTP[i] = new ExtendedButtonWidget(&tft);
  btnVerifyQRCode[0] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 4; i++) btnAdminMenu[i] = new ExtendedButtonWidget(&tft);
  for (int i = 0; i < 5; i++) btnOpenBoxAdminMenu[i] = new ExtendedButtonWidget(&tft);

  touch_calibrate();
  changeScreen(LANGUAGE_MENU);
}

void loop() {
  unsigned long currentMillis = millis();

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

  if (currentMillis - lastSerial2Read > 500) {
    lastSerial2Read = currentMillis;
    handleSerial2();
  }

  tft.unloadFont();
  yield();
}

void handleSerial2() {
  String *payload = readSerial2();

  if (payload == nullptr) {
    Serial.println("No data received or readSerial2() returned nullptr");
    return;
  }

  if (payload[0].length() == 0) {  // Check if the payload has no data
    Serial.println("Payload is empty");
    return;
  }

  if (payload[0] == "ADMIN") {
    Serial.println("Enter Admin Mode");
    changeScreen(ADMIN_MENU);
  } else if (payload[0] == "status") {
    for (int i = 0; i < sizeof(payload); i++) {
      displayMessage(payload[i + 1]);
      delay(1500);
    }
    changeScreen(ADMIN_MENU);
  } else if (payload[0] == "QR") {
    // show QR code
  }
}

String *readSerial2() {
  static String array[10];
  int arrayIndex = 0;
  unsigned long lastDataReceived = millis();

  char buffer[128] = { 0 };  // Statical allocate buffer and initialize to zero

  while (Serial2.available() > 0 && (millis() - lastDataReceived) < 500) {  // Increase timeout to 1000 ms
    char c = Serial2.read();
    Serial.write(c);              // Echo received data for testing (optional)
    lastDataReceived = millis();  // Update last data received time

    if (c == '\n') {
      if (strlen(buffer) > 0) {  // Check if buffer is not empty
        array[arrayIndex++] = String(buffer);
        buffer[0] = '\0';  // Reset buffer
      }
      if (arrayIndex == 10) {
        Serial.println("Serial2 array overflow");
        break;
      }
      return array;  // Return collected data
    } else if (c == ';') {
      if (strlen(buffer) > 0) {  // Check if buffer is not empty
        array[arrayIndex++] = String(buffer);
        buffer[0] = '\0';  // Reset buffer
      }
    } else {
      size_t len = strlen(buffer);
      if (len < sizeof(buffer) - 1) {  // Prevent buffer overflow
        buffer[len] = c;               // Append character to buffer safely
        buffer[len + 1] = '\0';        // Null-terminate
      } else {
        Serial.println("Serial2 buffer overflow");
        break;
      }
    }
    yield();
  }

  return nullptr;  // Return null if we haven't received a complete line
}

void writeSerial2(const String &data) {
  if (!Serial2) {
    Serial.println("writeSerial2 failed");
    return;  // Ensure Serial is initialized and available
  }

  for (char c : data) {
    Serial2.write(c);
  }
  Serial2.write('\n');
}

void handleLanguageMenu() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 0, t_y = 0;  // To store the touch coordinates

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    for (uint8_t b = 0; b < 2; b++) {
      if (pressed) {
        if (btnLanguageMenu[b]->contains(t_x, t_y)) {
          if (!btnLanguageMenu[b]->getState()) {
            // Only trigger the press action if the button wasn't already pressed
            btnLanguageMenu[b]->press(true);
            btnLanguageMenu[b]->pressAction();
          }
        }
      } else {
        // If the button is pressed but touch is released, trigger the release action
        if (btnLanguageMenu[b]->getState()) {
          btnLanguageMenu[b]->press(false);
          btnLanguageMenu[b]->releaseAction();
        }
      }
    }
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

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    for (uint8_t b = 0; b < 2; b++) {
      if (pressed) {
        if (btnVerifyMenu[b]->contains(t_x, t_y)) {
          if (!btnVerifyMenu[b]->getState()) {
            // Only trigger the press action if the button wasn't already pressed
            btnVerifyMenu[b]->press(true);
            btnVerifyMenu[b]->pressAction();
          }
        }
      } else {
        // If the button is pressed but touch is released, trigger the release action
        if (btnVerifyMenu[b]->getState()) {
          btnVerifyMenu[b]->press(false);
          btnVerifyMenu[b]->releaseAction();
        }
      }
    }
  }
}

void handleVerifyOTP(char *buffer, uint8_t length, uint16_t timeout) {
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

      // Debugging: Print touch coordinates to serial monitor
      // if (pressed) {
      //   Serial.print("Touch at: ");
      //   Serial.print(t_x);
      //   Serial.print(", ");
      //   Serial.println(t_y);
      // }

      for (uint8_t b = 0; b < 2; b++) {
        if (pressed) {
          if (btnVerifyOTP[b]->contains(t_x, t_y)) {
            if (!btnVerifyOTP[b]->getState()) {
              // Only trigger the press action if the button wasn't already pressed
              btnVerifyOTP[b]->press(true);
              btnVerifyOTP[b]->pressAction();
            }
          }
        } else {
          // If the button is pressed but touch is released, trigger the release action
          if (btnVerifyOTP[b]->getState()) {
            btnVerifyOTP[b]->press(false);
            btnVerifyOTP[b]->releaseAction();
          }
        }
      }
    }

    char key = keyPad.getChar();

    if (key != 'N' && key != 'F' && millis() - lastKeyPressTime > debounceDelay) {
      lastKeyPressTime = millis();

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

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    if (pressed) {
      if (btnVerifyQRCode[0]->contains(t_x, t_y)) {
        if (!btnVerifyQRCode[0]->getState()) {
          // Only trigger the press action if the button wasn't already pressed
          btnVerifyQRCode[0]->press(true);
          btnVerifyQRCode[0]->pressAction();
        }
      }
    } else {
      // If the button is pressed but touch is released, trigger the release action
      if (btnVerifyQRCode[0]->getState()) {
        btnVerifyQRCode[0]->press(false);
        btnVerifyQRCode[0]->releaseAction();
      }
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

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    for (uint8_t b = 0; b < 4; b++) {
      if (pressed) {
        if (btnAdminMenu[b]->contains(t_x, t_y)) {
          if (!btnAdminMenu[b]->getState()) {
            // Only trigger the press action if the button wasn't already pressed
            btnAdminMenu[b]->press(true);
            btnAdminMenu[b]->pressAction();
          }
        }
      } else {
        // If the button is pressed but touch is released, trigger the release action
        if (btnAdminMenu[b]->getState()) {
          btnAdminMenu[b]->press(false);
          btnAdminMenu[b]->releaseAction();
        }
      }
    }
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

    // Debugging: Print touch coordinates to serial monitor
    // if (pressed) {
    //   Serial.print("Touch at: ");
    //   Serial.print(t_x);
    //   Serial.print(", ");
    //   Serial.println(t_y);
    // }

    for (uint8_t b = 0; b < 5; b++) {
      if (pressed) {
        if (btnOpenBoxAdminMenu[b]->contains(t_x, t_y)) {
          if (!btnOpenBoxAdminMenu[b]->getState()) {
            // Only trigger the press action if the button wasn't already pressed
            btnOpenBoxAdminMenu[b]->press(true);
            btnOpenBoxAdminMenu[b]->pressAction();
          }
        }
      } else {
        // If the button is pressed but touch is released, trigger the release action
        if (btnOpenBoxAdminMenu[b]->getState()) {
          btnOpenBoxAdminMenu[b]->press(false);
          btnOpenBoxAdminMenu[b]->releaseAction();
        }
      }
    }
  }
}

void handleMessage() {
  // Handle message screen updates if needed
}

void changeScreen(ScreenState newScreen) {
  currentScreen = newScreen;
  tft.fillScreen(TFT_BLACK);

  // Clear all buttons
  for (int i = 0; i < 2; i++) btnLanguageMenu[i]->press(false);
  for (int i = 0; i < 2; i++) btnVerifyMenu[i]->press(false);
  for (int i = 0; i < 2; i++) btnVerifyOTP[i]->press(false);
  btnVerifyQRCode[0]->press(false);
  for (int i = 0; i < 4; i++) btnAdminMenu[i]->press(false);
  for (int i = 0; i < 5; i++) btnOpenBoxAdminMenu[i]->press(false);

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

void setupLanguageMenu() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2;
  // tft.setTextColor(TFT_LIGHTGREY);
  // tft.setCursor(50, 50);
  // tft.drawString("Choose a language",50,50);
  myfont.print(100, 50, "Choose a language:", TFT_LIGHTGREY, TFT_BLACK);

  btnLanguageMenu[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "EN", 1);
  btnLanguageMenu[0]->setPressAction(btnLanguageMenu_EN_pressAction);
  btnLanguageMenu[0]->setReleaseAction(btnLanguageMenu_EN_releaseAction);
  btnLanguageMenu[0]->drawSmoothButton(false, 3, TFT_BLACK);

  y = tft.height() / 2 + BUTTON_H + 10;
  btnLanguageMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "VN", 1);
  btnLanguageMenu[1]->setPressAction(btnLanguageMenu_VN_pressAction);
  btnLanguageMenu[1]->setReleaseAction(btnLanguageMenu_VN_releaseAction);
  btnLanguageMenu[1]->drawSmoothButton(false, 3, TFT_BLACK);
}

void setupVerifyMenu() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2;
  // tft.setTextColor(TFT_LIGHTGREY);
  // tft.setCursor(50, 50);

  if (language == "EN") {
    myfont.print(50, 50, "Select a verification method:", TFT_LIGHTGREY, TFT_BLACK);
  } else if (language == "VN") {
    myfont.print(50, 50, "Chọn phương thức xác thực:", TFT_LIGHTGREY, TFT_BLACK);
  }

  btnVerifyMenu[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "OTP", 1);
  btnVerifyMenu[0]->setPressAction(btnVerifyMenu_OTP_pressAction);
  btnVerifyMenu[0]->setReleaseAction(btnVerifyMenu_OTP_releaseAction);
  btnVerifyMenu[0]->drawSmoothButton(false, 3, TFT_BLACK);

  y = tft.height() / 2 + BUTTON_H + 10;
  btnVerifyMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "QR Code", 1);
  // btnVerifyMenu[1]->setPressAction(btnVerifyMenu_QRCode_pressAction);
  // btnVerifyMenu[1]->setReleaseAction(btnVerifyMenu_QRCode_releaseAction);
  btnVerifyMenu[1]->drawSmoothButton(false, 3, TFT_BLACK);
}

void setupVerifyOTP() {
  uint16_t x = (tft.width() - BUTTON_W) / 4;
  uint16_t y = tft.height() * 3 / 4;
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(50, 50);

  if (language == "EN") {
    myfont.print(100, 50, "Enter OTP", TFT_LIGHTGREY, TFT_BLACK);
  } else if (language == "VN") {
    myfont.print(100, 50, "Nhập mã OTP", TFT_LIGHTGREY, TFT_BLACK);
  }

  // Calculate the position to center the text field
  uint16_t fieldWidth = tft.width() / 2;
  uint16_t fieldHeight = 50;

  tft.fillRoundRect((tft.width() - fieldWidth) / 2, (tft.height() - fieldHeight) / 2, fieldWidth, 50, 5, 0xADD8E6);   // Draw a text field
  tft.drawRoundRect((tft.width() - fieldWidth) / 2, (tft.height() - fieldHeight) / 2, fieldWidth, 50, 5, TFT_WHITE);  // Add a border

  tft.setTextDatum(MC_DATUM);
  tft.drawString(userInput, tft.width() / 2, tft.height() / 2, 4);  // Center text inside the box

  btnVerifyOTP[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Submit", 1);
  btnVerifyOTP[0]->setPressAction(btnVerifyOTP_Submit_pressAction);
  btnVerifyOTP[0]->setReleaseAction(btnVerifyOTP_Submit_releaseAction);
  btnVerifyOTP[0]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnVerifyOTP[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "Cancel", 1);
  btnVerifyOTP[1]->setPressAction(btnVerifyOTP_Cancel_pressAction);
  btnVerifyOTP[1]->setReleaseAction(btnVerifyOTP_Cancel_releaseAction);
  btnVerifyOTP[1]->drawSmoothButton(false, 3, TFT_BLACK);
}

void setupVerifyQRCode() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() * 3 / 4;

  //Display QR Code
  // tft.setTextColor(TFT_LIGHTGREY);
  // tft.setCursor(50, 50);

  if (language == "EN") {
    myfont.print(100, 50, "Scan QR Code", TFT_LIGHTGREY, TFT_BLACK);
  } else if (language == "VN") {
    myfont.print(100, 50, "Quét mã QR", TFT_LIGHTGREY, TFT_BLACK);
  }

  btnVerifyQRCode[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "Cancel", 1);
  btnVerifyQRCode[0]->setPressAction(btnVerifyQRCode_Cancel_pressAction);
  btnVerifyQRCode[0]->setReleaseAction(btnVerifyQRCode_Cancel_releaseAction);
  btnVerifyQRCode[0]->drawSmoothButton(false, 3, TFT_BLACK);
}

void setupAdminMenu() {
  uint16_t x = (tft.width() - BUTTON_W) / 4;
  uint16_t y = tft.height() / 2;
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(50, 50);
  tft.println("Welcome to Admin Mode");

  btnAdminMenu[0]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Status", 1);
  btnAdminMenu[0]->setPressAction(btnAdminMenu_CheckDoorsStatus_pressAction);
  btnAdminMenu[0]->setReleaseAction(btnAdminMenu_CheckDoorsStatus_releaseAction);
  btnAdminMenu[0]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnAdminMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Open Box", 1);
  btnAdminMenu[1]->setPressAction(btnAdminMenu_OpenBox_pressAction);
  btnAdminMenu[1]->setReleaseAction(btnAdminMenu_OpenBox_releaseAction);
  btnAdminMenu[1]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) / 4;
  y = tft.height() / 2 + BUTTON_H + 10;
  btnAdminMenu[2]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Reset", 1);
  btnAdminMenu[2]->setPressAction(btnAdminMenu_Reset_pressAction);
  btnAdminMenu[2]->setReleaseAction(btnAdminMenu_Reset_releaseAction);
  btnAdminMenu[2]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnAdminMenu[3]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Exit", 1);
  btnAdminMenu[3]->setPressAction(btnAdminMenu_Exit_pressAction);
  btnAdminMenu[3]->setReleaseAction(btnAdminMenu_Exit_releaseAction);
  btnAdminMenu[3]->drawSmoothButton(false, 3, TFT_BLACK);
}

void setupOpenBoxAdminMenu() {
  uint16_t x = tft.width() * 3 / 4;
  uint16_t y = 50;
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(50, 50);
  tft.println("Open Box");

  btnOpenBoxAdminMenu[0]->initButtonUL(x, y, 100, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Back", 1);
  btnOpenBoxAdminMenu[0]->setPressAction(btnOpenBox_Back_pressAction);
  btnOpenBoxAdminMenu[0]->setReleaseAction(btnOpenBox_Back_releaseAction);
  btnOpenBoxAdminMenu[0]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) / 4;
  y = tft.height() / 2;
  btnOpenBoxAdminMenu[1]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Box 1", 1);
  btnOpenBoxAdminMenu[1]->setPressAction(btnOpenBox_Box1_pressAction);
  btnOpenBoxAdminMenu[1]->setReleaseAction(btnOpenBox_Box1_releaseAction);
  btnOpenBoxAdminMenu[1]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnOpenBoxAdminMenu[2]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Box 2", 1);
  btnOpenBoxAdminMenu[2]->setPressAction(btnOpenBox_Box2_pressAction);
  btnOpenBoxAdminMenu[2]->setReleaseAction(btnOpenBox_Box2_releaseAction);
  btnOpenBoxAdminMenu[2]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) / 4;
  y = tft.height() / 2 + BUTTON_H + 10;
  btnOpenBoxAdminMenu[3]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Box 3", 1);
  btnOpenBoxAdminMenu[3]->setPressAction(btnOpenBox_Box3_pressAction);
  btnOpenBoxAdminMenu[3]->setReleaseAction(btnOpenBox_Box3_releaseAction);
  btnOpenBoxAdminMenu[3]->drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W) * 3 / 4;
  btnOpenBoxAdminMenu[4]->initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_BLACK, "Box 4", 1);
  btnOpenBoxAdminMenu[4]->setPressAction(btnOpenBox_Box4_pressAction);
  btnOpenBoxAdminMenu[4]->setReleaseAction(btnOpenBox_Box4_releaseAction);
  btnOpenBoxAdminMenu[4]->drawSmoothButton(false, 3, TFT_BLACK);
}

void displayMessage(String message) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(50, tft.height() / 2 - 50);
  tft.println(message);
}

void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    } else {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void setupKeypad() {
  while (!keyPad.begin()) {
    displayMessage("ERROR\nPlease reboot.");
    Serial.println("ERROR: cannot communicate to keypad.\nPlease reboot.");
    ESP.restart();
    delay(1000);
  }

  keyPad.loadKeyMap(keymap);
  Serial.println("Keypad connected.");
}
