#ifndef DISPLAY_SETUP_H
#define DISPLAY_SETUP_H

#include <TFT_eSPI.h>
#include "FontMaker.h"

extern TFT_eSPI tft;
extern MakeFont myfont;

void initializeDisplay();
void touch_calibrate();
void setpx(int16_t x, int16_t y, uint16_t color);

#endif