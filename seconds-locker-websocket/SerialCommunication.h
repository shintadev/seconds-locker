#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <Arduino.h>

void handleSerial2();
String* readSerial2();
void writeSerial2(const String& data);

#endif
