#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define displaySCREEN_WIDTH 128
#define displaySCREEN_HEIGHT 64

/** Initialize pmodoled module */
void vDisplayInit();
/** Initialize SPI */
void spi_init(void);
/** write a byte to OLED spi */
void spi(uint8_t data);
/** wait for completion of queued spi bytes */
void spi_complete(void);
/** set mode to data */
void mode_data(void);
/** set mode to commands */
void mode_cmd(void);

void setDisplayOn(uint8_t on);
void setChargePumpVoltage(uint8_t voltage);
void invertColor(uint8_t invert);
void setEntireDisplayOn(uint8_t allWhite);
void setDisplayStartLine(uint8_t startline);
void setDisplayOffset(uint8_t something);
void flipDisplay(uint8_t flip);
void setContrast(uint8_t contrast);
void fadeIn(uint64_t millis);
void fadeOut(uint64_t millis);

void vSetPositionX(unsigned col);
void set_row(unsigned row);
void set_xrow(unsigned col, unsigned row);
/** clear (visible portion of) screen, reset pointers */
void oled_clear(void);

#endif
