#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

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

typedef enum PlayerMovement {
    eUp,
    eDown
} PlayerMovement;

typedef enum PlayerId {
    ePlayer1,
    ePlayer2
} PlayerId;

typedef struct Player_t {
    PlayerId eId;
    uint32_t ulX;
    uint32_t ulY;
} Player_t;

typedef struct Ball_t {
    uint32_t ulX;
    uint32_t ulY;
} Ball_t;

void vSystemInit(void);

void vSystemGetPlayerDefaultConfig(Player_t *pxPlayer, PlayerId eId);

void vSystemUpdatePlayerPosition(Player_t *pxPlayer, PlayerMovement eMovement);

void vSystemGetBallDefaultConfig(Ball_t *pxBall);

void vSystemDisplayDrawPlayer(const Player_t *pxPlayer);

void vSystemDisplayDrawBall(const Ball_t *pxBall);

void vSystemDisplayReset(void);

void vSystemDisplayUpdateFrame(void);

void vSystemSetLed(Led eLed);

void vSystemClearLed(Led eLed);

void vSystemToggleLed(Led eLed);

void vSystemWriteLed(Led eLed, Level eLevel);

Level eSystemReadButton(Button eButton);

#endif
