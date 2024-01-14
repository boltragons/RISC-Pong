#include "system.h"

#include "platform.h"
#include "wrap.h"
#include "startup.h"
#include "display.h"
#include "font.h"
#include "framebuffer.h"

#include "pin.h"

/* System Macros */

#define systemDISPLAY_HEIGHT       64
#define systemDISPLAY_WIDTH        128

#define systemPLAYER_HEIGHT        (systemDISPLAY_HEIGHT/5)
#define systemPLAYER_WIDTH         2 

#define systemPLAYER_DEFAULT_Y     ((systemDISPLAY_HEIGHT/2) - (systemPLAYER_HEIGHT/2))

#define systemPLAYER_1_DEFAULT_X   0
#define systemPLAYER_2_DEFAULT_X   (systemDISPLAY_WIDTH - systemPLAYER_WIDTH)

#define systemBALL_HEIGHT          2
#define systemBALL_WIDTH           2 

#define systemBALL_DEFAULT_Y       systemDISPLAY_HEIGHT/2
#define systemBALL_DEFAULT_X       systemDISPLAY_WIDTH/2

#define systemBUTTON_GREEN_PIN     18
#define systemBUTTON_BLUE_PIN      19
#define systemBUTTON_YELLOW_PIN    20
#define systemBUTTON_RED_PIN       21

#define systemLED_GREEN_PIN        22
#define systemLED_BLUE_PIN         23
#define systemLED_YELLOW_PIN       0
#define systemLED_RED_PIN          1

/* Pin Arrays */

uint32_t ulSystemLeds[eNumberLeds] = {systemLED_GREEN_PIN, systemLED_BLUE_PIN, systemLED_YELLOW_PIN, systemLED_RED_PIN};

uint32_t ulSystemButtons[eNumberButtons] = {systemBUTTON_GREEN_PIN, systemBUTTON_BLUE_PIN, systemBUTTON_YELLOW_PIN, systemBUTTON_RED_PIN};

/* Private Functions */

static void prvSystemLedInit(void);

static void prvSystemButtonInit(void);

static void prvSystemDisplayInit(void);

/* Global Functions */

void vSystemInit(void) {
    prvSystemLedInit();
    prvSystemButtonInit();
    prvSystemDisplayInit();
}

void vSystemGetPlayerDefaultConfig(Player_t *pxPlayer, PlayerId eId) {
    pxPlayer->eId = eId;
    pxPlayer->ulY = systemPLAYER_DEFAULT_Y;
    pxPlayer->ulX = (eId == ePlayer1)? systemPLAYER_1_DEFAULT_X : systemPLAYER_2_DEFAULT_X;
}

void vSystemUpdatePlayerPosition(Player_t *pxPlayer, PlayerMovement eMovement) {
    switch(eMovement) {
    case eUp:
        if (pxPlayer->ulY > 0) {
            pxPlayer->ulY--;
        }
        break;
    case eDown:
        if (pxPlayer->ulY < (systemDISPLAY_HEIGHT - systemPLAYER_HEIGHT)) {
            pxPlayer->ulY++;
        }
        break;
    }
}

void vSystemGetBallDefaultConfig(Ball_t *pxBall) {
    pxBall->ulY = systemBALL_DEFAULT_Y;
    pxBall->ulX = systemBALL_DEFAULT_X;
}

void vSystemDisplayDrawPlayer(const Player_t *pxPlayer) {
    vFrameBufferInit();
    for(int y = pxPlayer->ulY; y < pxPlayer->ulY + systemPLAYER_HEIGHT; y++) {
        for(int x = pxPlayer->ulX; x < pxPlayer->ulX + systemPLAYER_WIDTH; x++) {
            vFrameBufferSetPixelDirect(x, y, 1);
        }
    }
}

void vSystemDisplayDrawBall(const Ball_t *pxBall) {
    vFrameBufferInit();
    for(int y = pxBall->ulY; y < pxBall->ulY + systemBALL_HEIGHT; y++) {
        for(int x = pxBall->ulX; x < pxBall->ulX + systemBALL_WIDTH; x++) {
            vFrameBufferSetPixelDirect(x, y, 1);
        }
    }
}

void vSystemDisplayReset(void) {
    vFrameBufferInit();
}

void vSystemDisplayUpdateFrame(void) {
    vFrameBufferFlush();
}

void vSystemSetLed(Led eLed) {
    vPinSetLevel(ulSystemLeds[eLed]);
}

void vSystemClearLed(Led eLed) {
    vPinClearLevel(ulSystemLeds[eLed]);
}

void vSystemToggleLed(Led eLed) {
    vPinToggleLevel(ulSystemLeds[eLed]);
}

void vSystemWriteLed(Led eLed, Level eLevel) {
    vPinWriteLevel(ulSystemLeds[eLed], eLevel);
}

Level eSystemReadButton(Button eButton) {
    return !xPinReadLevel(ulSystemButtons[eButton]);
}

/* Private Functions */

void prvSystemDisplayInit(void) {
    vDisplayInit();
    vFrameBufferInit();
}
 
void prvSystemLedInit(void) {
    PinConfig_t xPinConfig = {
        .eDirection = eOutput,
        .xDefaultLevel = pinLOW,
        .xEnablePullUp = false
    };

    vPinInit(systemLED_GREEN_PIN, &xPinConfig);
    vPinInit(systemLED_BLUE_PIN, &xPinConfig);
    vPinInit(systemLED_YELLOW_PIN, &xPinConfig);
    vPinInit(systemLED_RED_PIN, &xPinConfig);
}
 
void prvSystemButtonInit(void) {
    PinConfig_t xPinConfig = {
        .eDirection = eInput,
        .xDefaultLevel = pinLOW,
        .xEnablePullUp = true
    };

    vPinInit(systemBUTTON_GREEN_PIN, &xPinConfig);
    vPinInit(systemBUTTON_BLUE_PIN, &xPinConfig);
    vPinInit(systemBUTTON_YELLOW_PIN, &xPinConfig);
    vPinInit(systemBUTTON_RED_PIN, &xPinConfig);
}
