#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <MFRC522.h>

extern MFRC522 rfid;

bool readRFID();

#endif
