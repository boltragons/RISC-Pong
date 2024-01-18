#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include "font.h"
#include "display.h"

#define framebufferCENTRALIZE_STRING(ulStringLength)\
    framebufferCENTRALIZE_STRING_BOX(ulStringLength, 0, displaySCREEN_WIDTH)

#define framebufferCENTRALIZE_STRING_BOX(ulStringLength, ucBoxX, ucBoxWidth)\
    (ucBoxX + (ucBoxWidth)/2 - (ulStringLength*fontCHAR_WIDTH)/2)

#define framebufferSTRING_WIDTH(ulStringLength)     (fontCHAR_WIDTH*ulStringLength)

#define framebufferSTRING_ROW(ulRow)                (8*ulRow)

#define framebufferCHAR_HEIGHT                      (8)

#include <stdint.h>

void vFrameBufferInit();

void vFrameBufferFlush();

uint8_t ucFrameBufferSetChar(uint8_t x, uint8_t y, uint8_t ucChar);

uint8_t ucFrameBufferSetCharInverted(uint8_t x, uint8_t y, uint8_t ucChar);

uint8_t ucFrameBufferSetString(uint8_t x, uint8_t y, const char *pcString);

uint8_t ucFrameBufferSetStringInverted(uint8_t x, uint8_t y, const char *pcString);

void vFrameBufferSetRectangle(uint8_t ucX, uint8_t ucY, uint8_t ucWidth, uint8_t ucHeight);

void vFrameBufferSetPixel(uint8_t x, uint8_t y);

void vFrameBufferSetPixelDirect(uint8_t x, uint8_t y);

uint8_t ucFrameBufferGetPixel(uint8_t x, uint8_t y);

#endif
