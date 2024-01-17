#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

#define fontWIDTH       6
#define fontHEIGHT      1
#define fontNUMBER      128
#define fontCHAR_WIDTH  (fontWIDTH + 1)

extern const uint8_t ucFonts[fontNUMBER][fontWIDTH];

void vFontPrintChar(uint8_t ch);

void vFontPrintText(const char* text);

void vFontClearScreen();

#endif
