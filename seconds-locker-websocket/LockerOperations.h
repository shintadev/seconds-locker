#ifndef LOCKER_OPERATIONS_H
#define LOCKER_OPERATIONS_H

#include <Adafruit_PWMServoDriver.h>

extern Adafruit_PWMServoDriver pwm;
extern int failCount;

void handleLockerOperations();
void openDoor(const String& doorId);
void openDoorAdmin(const int doorIndex);
bool checkDoorState(int pcf8574Addr, int pcfPin);
bool checkObject(int boxNumber, int pcf8574Addr, int pcfPinX, int pcfPinY);
void unlockBox(Adafruit_PWMServoDriver& pwm, int servoChannel);
void lockBox(Adafruit_PWMServoDriver& pwm, int servoChannel);
int angleToPulse(int angle);
void ringWarning();

#endif
