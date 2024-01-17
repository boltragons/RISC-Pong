#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <stdint.h>

void vFrameBufferInit();

void vFrameBufferFlush();

uint8_t ucFrameBufferSetChar(uint8_t x, uint8_t y, uint8_t ucChar);

uint8_t ucFrameBufferSetString(uint8_t x, uint8_t y, const char *pcString);

void vFrameBufferSetPixel(uint8_t x, uint8_t y);

void vFrameBufferSetPixelDirect(uint8_t x, uint8_t y);

uint8_t ucFrameBufferGetPixel(uint8_t x, uint8_t y);

#endif
