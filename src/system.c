#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Board Support Package (BSP) */
#include "platform.h"
#include "wrap.h"
#include "startup.h"

/* OLED Display Interface */
#include "display.h"
#include "font.h"
#include "framebuffer.h"

/* Hardware Abstraction Layer (HAL) */
#include "pin.h"

/* System Macros */

#define systemDISPLAY_HEIGHT       ( 64 )
#define systemDISPLAY_WIDTH        ( 128 )

#define systemPLAYER_HEIGHT        (systemDISPLAY_HEIGHT/5)
#define systemPLAYER_WIDTH         2 

#define systemPLAYER_DEFAULT_Y     ((systemDISPLAY_HEIGHT/2) - (systemPLAYER_HEIGHT/2))

#define systemPLAYER_OFFSET        6
#define systemPLAYER_1_DEFAULT_X   (systemPLAYER_OFFSET)
#define systemPLAYER_2_DEFAULT_X   (systemDISPLAY_WIDTH - systemPLAYER_WIDTH - systemPLAYER_OFFSET)

#define systemBALL_HEIGHT          2
#define systemBALL_WIDTH           2

#define systemBALL_DEFAULT_Y       ((systemDISPLAY_HEIGHT - systemBALL_HEIGHT)/2)
#define systemBALL_DEFAULT_X       ((systemDISPLAY_WIDTH - systemBALL_WIDTH)/2)

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

static void prvSystemMcuInit(void);

static void prvSystemLedInit(void);

static void prvSystemButtonInit(void);

static void prvSystemDisplayInit(void);

inline static void prvSystemDisplayDrawLine(void);

inline static void prvSystemDisplayDrawPlayer(const Player_t *pxPlayer);

inline static void prvSystemDisplayDrawBall(const Ball_t *pxBall);

inline static PlayerId prvSystemCheckWallColisionDetection(Ball_t *pxBall);

inline static bool prvSystemCheckPlayerColisionDetection(Ball_t *pxBall, const Player_t *pxPlayer01, const Player_t *pxPlayer02);

/* Global Functions */

void vSystemInit(void) {
    prvSystemMcuInit();
    prvSystemLedInit();
    prvSystemButtonInit();
    prvSystemDisplayInit();
}

void vSystemGetPlayerDefaultConfig(Player_t *pxPlayer, PlayerId eId) {
    pxPlayer->eId = eId;
    pxPlayer->ulY = systemPLAYER_DEFAULT_Y;
    pxPlayer->ulX = (eId == ePlayer1)? systemPLAYER_1_DEFAULT_X : systemPLAYER_2_DEFAULT_X;
    pxPlayer->ulPoints = 0;
    strcpy(pxPlayer->pcPointsString, "000");
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
    pxBall->ulVelocityX = systemBALL_DEFAULT_SPEED;
    pxBall->ulVelocityY = systemBALL_DEFAULT_SPEED;
}

void vSystemSetBallPosition(Ball_t *pxBall, PlayerId ePlayerToStart, bool xBallGoesUpwards) {
    pxBall->ulY = systemBALL_DEFAULT_Y;
    pxBall->ulX = systemBALL_DEFAULT_X;
    pxBall->ulVelocityX = (ePlayerToStart == ePlayer1)? -systemBALL_DEFAULT_SPEED : systemBALL_DEFAULT_SPEED;
    pxBall->ulVelocityY = (xBallGoesUpwards)? -systemBALL_DEFAULT_SPEED : systemBALL_DEFAULT_SPEED;
}

PlayerId vSystemUpdateBallPosition(Ball_t *pxBall, const Player_t *pxPlayer01, const Player_t *pxPlayer02) {
    pxBall->ulX += pxBall->ulVelocityX / systemDISPLAY_FRAMES_PER_SECOND;
    pxBall->ulY += pxBall->ulVelocityY / systemDISPLAY_FRAMES_PER_SECOND;

    if(prvSystemCheckPlayerColisionDetection(pxBall, pxPlayer01, pxPlayer02)) {
        return eNonePlayer;
    }

    return prvSystemCheckWallColisionDetection(pxBall);
}

void vSystemDisplayUpdateFrame(const Player_t *pxPlayer01, const Player_t *pxPlayer02, const Ball_t *pxBall) {
    vFrameBufferInit();
    prvSystemDisplayDrawLine();
    prvSystemDisplayDrawPlayer(pxPlayer01);
    prvSystemDisplayDrawPlayer(pxPlayer02);
    prvSystemDisplayDrawBall(pxBall);
    vFrameBufferFlush();
}

void vSystemDisplayMainScreen(void) {
    static const char *pcMainTitle = "RISC-Pong";
    const uint32_t ulMainTitleLength = strlen(pcMainTitle);

    static const char *pcMainTitleStart = "Press ANY Button";
    const uint32_t ulMainTitleStartLength = strlen(pcMainTitleStart);

    vFrameBufferInit();
    vFrameBufferSetRectangle(framebufferCENTRALIZE_STRING(ulMainTitleLength) - 10, framebufferSTRING_ROW(1), framebufferSTRING_WIDTH(ulMainTitleLength) + 20, framebufferSTRING_ROW(3));
    ucFrameBufferSetStringInverted(framebufferCENTRALIZE_STRING(ulMainTitleLength), framebufferSTRING_ROW(2), pcMainTitle);
    ucFrameBufferSetString(framebufferCENTRALIZE_STRING(ulMainTitleStartLength), framebufferSTRING_ROW(6), pcMainTitleStart);
    vFrameBufferFlush();
}

void vSystemDisplaySelectionScreen(void) {
    static const char *pcSelectTitle = "Who will serve?";
    static const uint32_t ulSelectTitleLength = 15;

    static const char *pcPlayer1Select = "P1";
    static const char *pcPlayer2Select = "P2";
    static const uint32_t ulPlayerSelectLength = 2;

    static const uint32_t ulPlayerSelect1BoxX = 0;
    static const uint32_t ulPlayerSelect2BoxX = displaySCREEN_WIDTH/2 + 10;
    static const uint32_t ulPlayerSelectBoxWidth = displaySCREEN_WIDTH/2 - 10;

    vFrameBufferInit();

    vFrameBufferSetRectangle(
            ulPlayerSelect1BoxX, 
            framebufferSTRING_ROW(5), 
            ulPlayerSelectBoxWidth, 
            framebufferSTRING_ROW(3)
    );

    vFrameBufferSetRectangle(
            ulPlayerSelect2BoxX, 
            framebufferSTRING_ROW(5), 
            ulPlayerSelectBoxWidth, 
            framebufferSTRING_ROW(3)
    );

    ucFrameBufferSetString(
            framebufferCENTRALIZE_STRING(ulSelectTitleLength), 
            framebufferSTRING_ROW(1), 
            pcSelectTitle
    );

    ucFrameBufferSetStringInverted(
            framebufferCENTRALIZE_STRING_BOX(ulPlayerSelectLength, ulPlayerSelect1BoxX, ulPlayerSelectBoxWidth), 
            framebufferSTRING_ROW(6), 
            pcPlayer1Select
    );

    ucFrameBufferSetStringInverted(
            framebufferCENTRALIZE_STRING_BOX(ulPlayerSelectLength, ulPlayerSelect2BoxX, ulPlayerSelectBoxWidth), 
            framebufferSTRING_ROW(6), 
            pcPlayer2Select
    );

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

uint32_t ulSystemBeginInterruptHandling(void) {
    return ulPinClaimInterrupt();
}

Button eSystemCheckInterruptSource(uint32_t ulInterruptId) {
    for(uint8_t i = 0; i < eNumberButtons; i++) {
        if(ulSystemButtons[i] == (intGPIO_IRQ_ID_TO_PIN(ulInterruptId))) {
            return i;
        }
    }
    return eInvalidButton;
}

void vSystemEndInterruptHandling(uint32_t ulInterruptId) {
    vPinClearInterruptFlag(intGPIO_IRQ_ID_TO_PIN(ulInterruptId));
    vPinCompleteInterrupt(ulInterruptId);
}

/* Private Functions */

static void prvSystemMcuInit(void) {
    for(uint32_t ulMcuPin = 0; ulMcuPin < 32; ulMcuPin++) {
        vPinInitDefaultConfig(ulMcuPin);
    }
    vInterruptsInitController();
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

    InterruptConfig_t xInterruptConfig = {
        .eEdgeTrigger = eBothEdges,
        .ucPriority = 1
    };

    vPinEnableInterrupt(systemBUTTON_GREEN_PIN, &xInterruptConfig);
    vPinEnableInterrupt(systemBUTTON_BLUE_PIN, &xInterruptConfig);
    vPinEnableInterrupt(systemBUTTON_YELLOW_PIN, &xInterruptConfig);
    vPinEnableInterrupt(systemBUTTON_RED_PIN, &xInterruptConfig);
}

ButtonEvent eSystemCheckButtonEvent(Button eButton) {
    InterruptTrigger eInterruptEvent = ePinCheckInterruptEvent(ulSystemButtons[eButton]);

    if(eInterruptEvent == eFallingEdge) {
        return ePressed;
    }
    else if(eInterruptEvent == eRisingEdge) {
        return eReleased;
    }

    return eNoneEvent;
}

void prvSystemDisplayInit(void) {
    vDisplayInit();
    vFrameBufferInit();
}

inline static void prvSystemDisplayDrawLine(void) {
    for(int y = 0; y < systemDISPLAY_HEIGHT; y += 6) {
        vFrameBufferSetPixel(systemDISPLAY_WIDTH/2 - 1, y);
    }
}

inline static void prvSystemDisplayDrawPlayer(const Player_t *pxPlayer) {
    vFrameBufferSetRectangle(pxPlayer->ulX, pxPlayer->ulY, systemPLAYER_WIDTH, systemPLAYER_HEIGHT);

    uint32_t ulScoreX = (pxPlayer->eId == ePlayer1)? 0 : (systemDISPLAY_WIDTH - 3*fontCHAR_WIDTH);
    ucFrameBufferSetString(ulScoreX, 0, pxPlayer->pcPointsString);
}

inline static void prvSystemDisplayDrawBall(const Ball_t *pxBall) {
    vFrameBufferSetRectangle(pxBall->ulX, pxBall->ulY, systemBALL_WIDTH, systemBALL_HEIGHT);
}

inline static PlayerId prvSystemCheckWallColisionDetection(Ball_t *pxBall) {
    PlayerId xWinnerPlayer = eNonePlayer;

    if(pxBall->ulX <= 0) {
        xWinnerPlayer = ePlayer2;
    }
    else if(pxBall->ulX >= (systemDISPLAY_WIDTH - systemBALL_WIDTH)) {
        xWinnerPlayer = ePlayer1;
    }

    if(pxBall->ulY <= 0) {
        pxBall->ulY = 0;
        pxBall->ulVelocityY = -pxBall->ulVelocityY;
    } else if(pxBall->ulY >= (systemDISPLAY_HEIGHT - systemBALL_HEIGHT)) {
        pxBall->ulY = (systemDISPLAY_HEIGHT - systemBALL_HEIGHT);
        pxBall->ulVelocityY = -pxBall->ulVelocityY;
    }

    return xWinnerPlayer;
}

inline static bool prvSystemCheckPlayerColisionDetection(Ball_t *pxBall, const Player_t *pxPlayer01, const Player_t *pxPlayer02) {
    bool xColisionDetected = false;

    if((pxBall->ulX <= pxPlayer01->ulX + systemPLAYER_WIDTH)
            && ((pxBall->ulY + systemBALL_HEIGHT - 1 >= pxPlayer01->ulY) && (pxBall->ulY + systemBALL_HEIGHT - 1 <= pxPlayer01->ulY + systemPLAYER_HEIGHT))) {
        pxBall->ulX = pxPlayer01->ulX + systemPLAYER_WIDTH;
        pxBall->ulVelocityX = -pxBall->ulVelocityX;
        xColisionDetected = true;
    }
    else if((pxBall->ulX + systemBALL_WIDTH >= pxPlayer02->ulX)
            && ((pxBall->ulY + systemBALL_HEIGHT - 1 >= pxPlayer02->ulY) && (pxBall->ulY + systemBALL_HEIGHT - 1 <= pxPlayer02->ulY + systemPLAYER_HEIGHT))) {
        pxBall->ulX = pxPlayer02->ulX - systemBALL_WIDTH;
        pxBall->ulVelocityX = -pxBall->ulVelocityX;
        xColisionDetected = true;
    }

    return xColisionDetected;
}
