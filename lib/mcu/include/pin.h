#ifndef PIN_H
#define PIN_H

#include <stdint.h>
#include <stdbool.h>

#include "interrupts.h"

#define pinLOW      0
#define pinHIGH     1

typedef uint8_t DigitalLevel;

typedef enum PinDirection {
    eInput,
    eOutput
} PinDirection;

typedef struct PinConfig_t {
    PinDirection eDirection;
    DigitalLevel xDefaultLevel;
    bool xEnablePullUp;
} PinConfig_t;

typedef enum InterruptTrigger {
    eNoneEdge = 1,
    eRisingEdge,
    eFallingEdge,
    eBothEdges
} InterruptTrigger;

typedef struct InterruptConfig_t {
    InterruptTrigger eEdgeTrigger;
    uint8_t ucPriority;
} InterruptConfig_t;

void vPinInit(uint32_t ulPinNumber, const PinConfig_t *xPinConfig);

void vPinInitDefaultConfig(uint32_t ulPinNumber);

void vPinEnableInterrupt(uint32_t ulPinNumber, const InterruptConfig_t *xInterruptConfig);

void vPinGetDefaultInterruptConfig(InterruptConfig_t *xInterruptConfig);

InterruptTrigger ePinCheckInterruptEvent(uint32_t ulPinNumber);

void vPinClearInterruptFlag(uint32_t ulPinNumber);

void vPinSetLevel(uint32_t ulPinNumber);

void vPinClearLevel(uint32_t ulPinNumber);

void vPinToggleLevel(uint32_t ulPinNumber);

void vPinWriteLevel(uint32_t ulPinNumber, DigitalLevel xLevel);

DigitalLevel xPinReadLevel(uint32_t ulPinNumber);

static inline uint32_t ulPinClaimInterrupt(void) {
    return ulInterruptsClaim();
}

static inline void vPinCompleteInterrupt(uint32_t ulInterruptId) {
    vInterruptsComplete(ulInterruptId);
}

#endif
