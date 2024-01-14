#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <stdint.h>

void vFrameBufferInit();
void vFrameBufferFlush();

void vFrameBufferSetPixel(uint8_t x, uint8_t y, uint8_t p);
void vFrameBufferSetPixelDirect(uint8_t x, uint8_t y, uint8_t p);
uint8_t fb_get_pixel(uint8_t x, uint8_t y);

#endif
