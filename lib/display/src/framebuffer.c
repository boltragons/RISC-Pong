// Original code from https://github.com/agra-uni-bremen/sifive-hifive1
/*
 * framebuffer.c
 *
 *  Created on: Feb 3, 2020
 *      Author: dwd
 */
#include "framebuffer.h"

#include <string.h>

#include "font.h"
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

uint8_t ucFrameBufferSetChar(uint8_t x, uint8_t y, uint8_t ucChar){
	for(int i = 0; i < fontWIDTH; i++) {
		ucFrameBuffer[x++][y/8] |= ucFonts[ucChar][i] << y%8;
    }
	ucFrameBuffer[x++][y/8] &= ~(1 << y%8);
	return x;
}

uint8_t ucFrameBufferSetCharInverted(uint8_t x, uint8_t y, uint8_t ucChar){
	for(int i = 0; i < fontWIDTH; i++) {
		ucFrameBuffer[x++][y/8] &= ~(ucFonts[ucChar][i] << y%8);
    }
	ucFrameBuffer[x++][y/8] |= (1 << y%8);
	return x;
}

uint8_t ucFrameBufferSetString(uint8_t x, uint8_t y, const char *pcString) {
	uint8_t ucLastX = x;
	for(int i = 0; i < strlen(pcString); i++) {
		ucLastX = ucFrameBufferSetChar(ucLastX, y, pcString[i]);
	}
	return ucLastX;
}

uint8_t ucFrameBufferSetStringInverted(uint8_t x, uint8_t y, const char *pcString) {
	uint8_t ucLastX = x;
	for(int i = 0; i < strlen(pcString); i++) {
		ucLastX = ucFrameBufferSetCharInverted(ucLastX, y, pcString[i]);
	}
	return ucLastX;
}

void vFrameBufferSetRectangle(uint8_t ucX, uint8_t ucY, uint8_t ucWidth, uint8_t ucHeight) {
	for(uint32_t ulDrawY = ucY; ulDrawY < ucY + ucHeight; ulDrawY++) {
        for(uint32_t ulDrawX = ucX; ulDrawX < ucX + ucWidth; ulDrawX++) {
            vFrameBufferSetPixel(ulDrawX, ulDrawY);
        }
    }
}

void vFrameBufferSetPixel(uint8_t x, uint8_t y) {
	ucFrameBuffer[x][y/8] |= 1 << y%8;
}

void vFrameBufferSetPixelDirect(uint8_t x, uint8_t y) {
	vFrameBufferSetPixel(x, y);
	set_xrow(x, y/8);
	spi(ucFrameBuffer[x][y/8]);
}


uint8_t ucFrameBufferGetPixel(uint8_t x, uint8_t y) {
	return ucFrameBuffer[x][y/8] & (1 << y%8);
}

