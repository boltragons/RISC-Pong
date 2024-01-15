#include "pin.h"

#include "platform.h"

void vPinInit(uint32_t ulPinNumber, const PinConfig_t *xPinConfig) {
    GPIO_REG(GPIO_IOF_EN) &= ~(1U << ulPinNumber);
    
    if(xPinConfig->xEnablePullUp) {
        GPIO_REG(GPIO_PULLUP_EN) |= (1U << ulPinNumber);
    }
    else {
        GPIO_REG(GPIO_PULLUP_EN) &= ~(1U << ulPinNumber);
    }

    if(xPinConfig->eDirection == eOutput) {
        GPIO_REG(GPIO_OUTPUT_EN) |= (1U << ulPinNumber);
        GPIO_REG(GPIO_INPUT_EN)  &= ~(1U << ulPinNumber);

        if(xPinConfig->xDefaultLevel == pinHIGH) {
            GPIO_REG(GPIO_OUTPUT_VAL) |= (1U << ulPinNumber);
        }
        else {
            GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1U << ulPinNumber);
        }
    }
    else {
        GPIO_REG(GPIO_INPUT_EN)  |= (1U << ulPinNumber);
        GPIO_REG(GPIO_OUTPUT_EN) &= ~(1U << ulPinNumber);
    }
}

void vPinInitDefaultConfig(uint32_t ulPinNumber) {
    GPIO_REG(GPIO_IOF_EN)    &= ~(1U << ulPinNumber);
    GPIO_REG(GPIO_INPUT_EN)  |=  (1U << ulPinNumber);
    GPIO_REG(GPIO_OUTPUT_EN) &= ~(1U << ulPinNumber);
    GPIO_REG(GPIO_PULLUP_EN) |=  (1U << ulPinNumber);
}

void vPinEnableInterrupt(uint32_t ulPinNumber, const InterruptConfig_t *xInterruptConfig) {
    switch (xInterruptConfig->eEdgeTrigger) {
    case eRisingEdge:
        GPIO_REG(GPIO_RISE_IE) |= (1U << ulPinNumber);
        GPIO_REG(GPIO_FALL_IE) &= ~(1U << ulPinNumber);
        break;
    case eFallingEdge:
        GPIO_REG(GPIO_FALL_IE) |= (1U << ulPinNumber);
        GPIO_REG(GPIO_RISE_IE) &= ~(1U << ulPinNumber);
        break;
    case eBothEdges:
        GPIO_REG(GPIO_RISE_IE) |= (1U << ulPinNumber);
        GPIO_REG(GPIO_FALL_IE) |= (1U << ulPinNumber);
        break;
    default:
        return;
    }
    
    vInterruptsEnable(intGPIO_IRQ_ID(ulPinNumber), xInterruptConfig->ucPriority);
}

void vPinGetDefaultInterruptConfig(InterruptConfig_t *xInterruptConfig) {
    xInterruptConfig->eEdgeTrigger = eRisingEdge;
    xInterruptConfig->ucPriority = 0;
}

InterruptTrigger ePinCheckInterruptEvent(uint32_t ulPinNumber) {
    if(GPIO_REG(GPIO_FALL_IP) & (1U << ulPinNumber)) {
        return eFallingEdge;
    } else if(GPIO_REG(GPIO_RISE_IP) & (1U << ulPinNumber)) {
        return eRisingEdge;
    }
    return eNoneEdge;
}

void vPinClearInterruptFlag(uint32_t ulPinNumber) {
    GPIO_REG(GPIO_FALL_IP) |= (1U << ulPinNumber);
    GPIO_REG(GPIO_RISE_IP) |= (1U << ulPinNumber);
}

void vPinSetLevel(uint32_t ulPinNumber) {
    GPIO_REG(GPIO_OUTPUT_VAL) |= (1U << ulPinNumber);
}

void vPinClearLevel(uint32_t ulPinNumber) {
    GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1U << ulPinNumber);
}

void vPinToggleLevel(uint32_t ulPinNumber) {
    GPIO_REG(GPIO_OUTPUT_VAL) ^= (1U << ulPinNumber);
}

void vPinWriteLevel(uint32_t ulPinNumber, DigitalLevel xLevel) {
    if(xLevel == pinHIGH) {
        GPIO_REG(GPIO_OUTPUT_VAL) |= (1U << ulPinNumber);
    }
    else {
        GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1U << ulPinNumber);
    }
}

DigitalLevel xPinReadLevel(uint32_t ulPinNumber) {
    return (GPIO_REG(GPIO_INPUT_VAL) >> ulPinNumber) & 1;
}

