#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

void vFontPrintChar(uint8_t ch);
void vFontPrintText(const char* text);
void vFontClearScreen();

#endif
