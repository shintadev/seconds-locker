#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <Arduino.h>

extern uint32_t serial2Timeout;

void handleSerial2();
String* readSerial2();
void writeSerial2(const String& data);

#endif
