#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

/* System Macros */

#define systemNUMBER_PLAYERS            2
#define systemDISPLAY_FRAME_PER_SECOND  60
#define systemDISPLAY_FRAME_DURATION    16

/* System Enums */

typedef enum Level {
    eLow,
    eHigh
} Level;

typedef enum Button {
    eInvalidButton = -1,
    eGreenButton,
    eBlueButton,
    eYellowButton,
    eRedButton,
    eNumberButtons
} Button;

typedef enum Led {
    eInvalidLed = -1,
    eGreenLed,
    eBlueLed,
    eYellowLed,
    eRedLed,
    eNumberLeds
} Led;

typedef enum ButtonEvent {
    eNoneEvent = -1,
    eReleased,
    ePressed
} ButtonEvent;

typedef enum PlayerMovement {
    eUp,
    eDown
} PlayerMovement;

typedef enum PlayerId {
    ePlayer1,
    ePlayer2
} PlayerId;

/* System Structs */

typedef struct Player_t {
    PlayerId eId;
    uint32_t ulX;
    uint32_t ulY;
} Player_t;

typedef struct Ball_t {
    uint32_t ulX;
    uint32_t ulY;
} Ball_t;

/* Initialization Interface */

void vSystemInit(void);

/* Game Objects Interface */

void vSystemGetPlayerDefaultConfig(Player_t *pxPlayer, PlayerId eId);

void vSystemUpdatePlayerPosition(Player_t *pxPlayer, PlayerMovement eMovement);

void vSystemGetBallDefaultConfig(Ball_t *pxBall);

/* Display Interface */

void vSystemDisplayUpdateFrame(const Player_t *pxPlayer01, const Player_t *pxPlayer02, const Ball_t *pxBall);

/* LED Interface */

void vSystemSetLed(Led eLed);

void vSystemClearLed(Led eLed);

void vSystemToggleLed(Led eLed);

void vSystemWriteLed(Led eLed, Level eLevel);

/* Button Interface */

Level eSystemReadButton(Button eButton);

/* Interrupt Interface */

uint32_t ulSystemBeginInterruptHandling(void);

Button eSystemCheckInterruptSource(uint32_t ulInterruptId);

ButtonEvent eSystemCheckButtonEvent(Button eButton);

void vSystemEndInterruptHandling(uint32_t ulInterruptId);

#endif
