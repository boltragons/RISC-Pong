#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define intGPIO_IRQ_ID(pin) (8 + pin)

#define intGPIO_IRQ_ID_TO_PIN(id) (id - 8)

#define intCHECK_GPIO_IRQ_SOURCE(id) (id >=8 && id <= 39)

void vInterruptsInitController(void);

void vInterruptsEnable(uint32_t ulInterruptId, uint8_t ucPriority);

uint32_t ulInterruptsClaim(void);

void vInterruptsComplete(uint32_t ulInterruptId);

#endif
