#include "interrupts.h"

#include "platform.h"

void vInterruptsInitController(void) {
    PLIC_REG(PLIC_ENABLE_OFFSET) = 0;
    PLIC_REG(PLIC_ENABLE_OFFSET + 4) = 0;
    PLIC_REG(PLIC_THRESHOLD_OFFSET) = 0;
}

void vInterruptsEnable(uint32_t ulInterruptId, uint8_t ucPriority) {
    if(ulInterruptId < 32) {
        PLIC_REG(PLIC_ENABLE_OFFSET) |= (1U << ulInterruptId);
    }
    else {
        PLIC_REG(PLIC_ENABLE_OFFSET + 4) |= (1U << (ulInterruptId - 32));
    }
    PLIC_REG(4*ulInterruptId) = ucPriority;
}

uint32_t ulInterruptsClaim(void) {
    return PLIC_REG(PLIC_CLAIM_OFFSET);
}

void vInterruptsComplete(uint32_t ulInterruptId) {
    PLIC_REG(PLIC_CLAIM_OFFSET) = ulInterruptId;
}