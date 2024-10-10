#ifndef TOKEN_MANAGER_H
#define TOKEN_MANAGER_H

#include <Arduino.h>

extern String token;

void saveToken();
void loadToken();
void clearEEPROM();

#endif
