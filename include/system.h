#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stdbool.h> 

/* System Macros */

#define systemNUMBER_PLAYERS                ( 2 )
#define systemDISPLAY_FRAMES_PER_SECOND     ( 60 ) 
#define systemDISPLAY_FRAME_DURATION_MS     ( 1000/systemDISPLAY_FRAMES_PER_SECOND )

#define systemDEBOUNCING_TIME_MS            ( 20 )

#define systemBALL_DEFAULT_SPEED            ( 200 )

#define systemPLAYER_POINTS_STRING_LENGTH   ( 2 )

#define systemPLAYER_POINTS_TO_WIN          ( 11 )

#define systemMINIMUM_OF_VICTORIES          ( 2 )

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
    eNonePlayer = -1,
    ePlayer1,
    ePlayer2
} PlayerId;

/* System Structs */

typedef struct Player_t {
    PlayerId eId;
    uint32_t ulX;
    uint32_t ulY;
    uint8_t ucPoints;
    uint8_t ucWins;
    char pcPointsString[systemPLAYER_POINTS_STRING_LENGTH + 2];
} Player_t;

typedef struct Ball_t {
    int32_t ulX;
    int32_t ulY;
    int32_t ulVelocityX;
    int32_t ulVelocityY;
} Ball_t;

/* Initialization Interface */

void vSystemInit(void);

/* Game Objects Interface */

void vSystemGetPlayerDefaultConfig(Player_t *pxPlayer, PlayerId eId);

void vSystemUpdatePlayerPosition(Player_t *pxPlayer, PlayerMovement eMovement);

void vSystemGetBallDefaultConfig(Ball_t *pxBall);

void vSystemSetBallPosition(Ball_t *pxBall, PlayerId ePlayerToStart, bool xBallGoesUpwards);

PlayerId vSystemUpdateBallPosition(Ball_t *pxBall, const Player_t *pxPlayer01, const Player_t *pxPlayer02);

/* Display Interface */

void vSystemDisplayUpdateFrame(const Player_t *pxPlayer01, const Player_t *pxPlayer02, const Ball_t *pxBall);

void vSystemDisplayMainScreen(void);

void vSystemDisplaySelectionScreen(void);

void vSystemDisplayGameOverScreen(PlayerId eWinner);

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
