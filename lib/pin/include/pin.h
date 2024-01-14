#ifndef PIN_H
#define PIN_H

#include <stdint.h>
#include <stdbool.h>

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

typedef enum EdgeTrigger {
    eRisingEdge,
    eFallingEdge,
    eBothEdges
} InterruptTrigger;

typedef struct InterruptConfig_t {
    InterruptTrigger eEdgeTrigger;
    uint8_t ucPriority;
} InterruptConfig_t;

void vPinInit(uint32_t ulPinNumber, const PinConfig_t *xPinConfig);

void vPinGetDefaultConfig(PinConfig_t *xPinConfig);

void vPinEnableInterrupt(uint32_t ulPinNumber, const InterruptConfig_t *xInterruptConfig);

void vPinGetDefaultInterruptConfig(InterruptConfig_t *xInterruptConfig);

uint32_t ulPinClaimInterrupt(void);

void vPinCompleteInterrupt(uint32_t ulInterruptId);

void vPinClearInterruptFlag(uint32_t ulPinNumber);

void vPinSetLevel(uint32_t ulPinNumber);

void vPinClearLevel(uint32_t ulPinNumber);

void vPinToggleLevel(uint32_t ulPinNumber);

void vPinWriteLevel(uint32_t ulPinNumber, DigitalLevel xLevel);

DigitalLevel xPinReadLevel(uint32_t ulPinNumber);

#endif
