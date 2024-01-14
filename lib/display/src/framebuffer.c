// Original code from https://github.com/agra-uni-bremen/sifive-hifive1
/*
 * framebuffer.c
 *
 *  Created on: Feb 3, 2020
 *      Author: dwd
 */
#include "framebuffer.h"
#include <string.h>

#include "display.h"

uint8_t ucFrameBuffer[displaySCREEN_WIDTH][displaySCREEN_HEIGHT/8];

void vFrameBufferInit() {
	memset(ucFrameBuffer, 0, displaySCREEN_WIDTH*(displaySCREEN_HEIGHT/8));
}

void vFrameBufferFlush() {
	for(uint8_t y = 0; y < displaySCREEN_HEIGHT/8; y++)
	{
		set_xrow(0, y);
		for(uint8_t x = 0; x < displaySCREEN_WIDTH; x++)
		{
			spi(ucFrameBuffer[x][y]);
		}
	}
}

void vFrameBufferSetPixel(uint8_t x, uint8_t y, uint8_t p) {
	if(p)
		ucFrameBuffer[x][y/8] |= 1 << y%8;
	else
		ucFrameBuffer[x][y/8] &= ~(1 << y%8);
}

void vFrameBufferSetPixelDirect(uint8_t x, uint8_t y, uint8_t p) {
	vFrameBufferSetPixel(x, y, p);
	set_xrow(x, y/8);
	spi(ucFrameBuffer[x][y/8]);
}


uint8_t fb_get_pixel(uint8_t x, uint8_t y) {
	return ucFrameBuffer[x][y/8] & (1 << y%8);
}

