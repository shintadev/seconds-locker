#include "RFIDHandler.h"

bool readRFID() {
  String tag = "";
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    if (tag == "991711115") {
      Serial.println("Enter admin mode...");
      return true;
    } else {
      Serial.println("Illegal access attempt");
    }
  }
  return false;
}
