#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h> 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "system.h"

/* Useful Macros */

#define assert(x) configASSERT(x)
            
/* Communication Handles Types */

typedef struct QueuePlayerItem_t {
    PlayerId ePlayer;
    PlayerMovement eMovement;
} QueuePlayerItem_t;

typedef struct QueueScoreItem_t {
    PlayerId eWinner;
} QueueScoreItem_t;

typedef struct PlayerPad_t {
    Button eButton;
    ButtonEvent eEvent;
} PlayerPad_t;

typedef enum GamePhase {
    eInitalPhase,
    eSelectionPhase,
    eStartPhase,
    ePlayPhase,
    eEndPhase
} GamePhase;
            
/* Synchronization Handles */

QueueHandle_t xQueuePlayer;

QueueHandle_t xQueueScore;

SemaphoreHandle_t xSemaphorePhase;

SemaphoreHandle_t xSemaphoreButton;

SemaphoreHandle_t xSemaphoreDisplay;
            
/* Communication Handles */

PlayerPad_t xPlayerPad1;

PlayerPad_t xPlayerPad2;

Player_t xPlayer01;

Player_t xPlayer02;

Ball_t xBall;

GamePhase eCurrentGamePhase;
            
/* Timers Handles */

TimerHandle_t xTimerDisplayRender;

TimerHandle_t xTimerSoftwareDebouncing;
            
/* Tasks Handles */

TaskHandle_t xGamePhaseHandlingTaskHandle;

TaskHandle_t xUpdateDisplayTaskHandle;

TaskHandle_t xInputHandlingTaskHandle;

TaskHandle_t xUpdateScoreTaskHandle;

TaskHandle_t xUpdatePlayerPositionTaskHandle;
            
/* Tasks Procedures */
            
void vGamePhaseHandlingTask(void *pvParameters);
            
void vUpdateDisplayTask(void *pvParameters);
            
void vInputHandlingTask(void *pvParameters);
            
void vUpdateScoreTask(void *pvParameters);
            
void vUpdatePlayerPositionTask(void *pvParameters);

/* Timers Callbacks */

void vRenderTimerCallback(TimerHandle_t xTimer);

void vSoftwareDebouncingTimerCallback(TimerHandle_t xTimer);

/* Private Interrupt Handlers */

BaseType_t prvButtonInterruptHandler(Button eButton);

/* Private Configuration Functions */

static inline void prvSystemConfig(void);

static inline void prvKernelConfig(void);

static inline void prvKernelStart(void);

static inline void prvKernelResume(void);

static inline void prvKernelPause(void);

/* Main Code */

int main(void) {
    prvSystemConfig();

    prvKernelConfig();

    prvKernelStart();

    while(1);
}       
                  
/* Tasks Procecudures */

void vGamePhaseHandlingTask(void *pvParameters) {
    while(1) {
        portENTER_CRITICAL();
        if(eCurrentGamePhase != eStartPhase) {
            portDISABLE_INTERRUPTS();
            xTimerStart(xTimerSoftwareDebouncing, 0);
        }

        switch(eCurrentGamePhase) {
        case eInitalPhase:
            vSystemDisplayMainScreen();
            eCurrentGamePhase = eSelectionPhase;
            break;
        case eSelectionPhase:
            vSystemDisplaySelectionScreen();
            vSystemGetPlayerDefaultConfig(&xPlayer01, ePlayer1);
            vSystemGetPlayerDefaultConfig(&xPlayer02, ePlayer2);
            eCurrentGamePhase = eStartPhase;
            break;
        case eStartPhase:
            if(xPlayerPad1.eEvent == ePressed) {
                vSystemSetBallPosition(&xBall, ePlayer1, (xPlayerPad1.eButton == eGreenButton));
            }
            else {
                vSystemSetBallPosition(&xBall, ePlayer2, (xPlayerPad1.eButton == eRedButton));
            }
            prvKernelResume();
            eCurrentGamePhase = ePlayPhase;
            break;
        case eEndPhase:
            prvKernelPause();
            if(xPlayer01.ucWins > xPlayer02.ucWins) {
                vSystemDisplayGameOverScreen(ePlayer1);
            }
            else {
                vSystemDisplayGameOverScreen(ePlayer2);
            }
            eCurrentGamePhase = eInitalPhase;
            break;
        default:
            break;
        }
        portEXIT_CRITICAL();

        xSemaphoreTake(xSemaphorePhase, portMAX_DELAY);
    }
}
        
void vUpdateDisplayTask(void *pvParameters) {
    QueueScoreItem_t xQueueScoreItem;

    vTaskSuspend(NULL);

    while(1) {
        xSemaphoreTake(xSemaphoreDisplay, portMAX_DELAY);

        portENTER_CRITICAL();
        vSystemDisplayUpdateFrame(&xPlayer01, &xPlayer02, &xBall);
        xQueueScoreItem.eWinner = vSystemUpdateBallPosition(&xBall, &xPlayer01, &xPlayer02);

        if(xQueueScoreItem.eWinner != eNonePlayer) {
            TickType_t xCurrentTickCount = xTaskGetTickCount();
            vSystemSetBallPosition(&xBall, !xQueueScoreItem.eWinner, (bool) (xCurrentTickCount % 2));
            xQueueSendToBack(xQueueScore, &xQueueScoreItem, 0);
        }
        portEXIT_CRITICAL();
    }
}
            
void vInputHandlingTask(void *pvParameters) {
    QueuePlayerItem_t xQueuePlayerItem;

    xPlayerPad1.eEvent = eReleased;
    xPlayerPad2.eEvent = eReleased;

    PlayerPad_t *pxPlayersPads[] = {&xPlayerPad1, &xPlayerPad2};

    vTaskSuspend(NULL);

    while(1) {
        if(xPlayerPad1.eEvent == ePressed || xPlayerPad2.eEvent == ePressed) {
            xSemaphoreTake(xSemaphoreButton, pdMS_TO_TICKS(0));
        }
        else {
            xSemaphoreTake(xSemaphoreButton, portMAX_DELAY);
        }

        for(uint32_t ulPad = 0; ulPad < systemNUMBER_PLAYERS; ulPad++) {
            portENTER_CRITICAL();
            if(pxPlayersPads[ulPad]->eEvent == ePressed) {
                xQueuePlayerItem.ePlayer = ulPad;
                if(pxPlayersPads[ulPad]->eButton == eGreenButton || pxPlayersPads[ulPad]->eButton == eRedButton) {
                    xQueuePlayerItem.eMovement = eUp;
                }
                else {
                    xQueuePlayerItem.eMovement = eDown;
                }
                xQueueSendToBack(xQueuePlayer, (void *) &xQueuePlayerItem, portMAX_DELAY);
            }
            portEXIT_CRITICAL();
        }

        /* Software Debouncing */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}  
            
void vUpdateScoreTask(void *pvParameters) {
    QueueScoreItem_t QueueScoreItem;

    vTaskSuspend(NULL);

    while(1) {
        xQueueReceive(xQueueScore, (void *) &QueueScoreItem, portMAX_DELAY);

        portENTER_CRITICAL();
        switch(QueueScoreItem.eWinner) {
        case ePlayer1:
            xPlayer01.ucPoints++;
            if(xPlayer01.ucPoints == systemPLAYER_POINTS_TO_WIN) {
                xPlayer01.ucPoints = 0;
                xPlayer01.ucWins++;

                if(xPlayer01.ucWins == systemMINIMUM_OF_VICTORIES) {
                    eCurrentGamePhase = eEndPhase;
                    xSemaphoreGive(xSemaphorePhase);
                }
            }
            sprintf(xPlayer01.pcPointsString, "%02d", xPlayer01.ucPoints);
            break;
        case ePlayer2:
            xPlayer02.ucPoints++;
            if(xPlayer02.ucPoints == systemPLAYER_POINTS_TO_WIN) {
                xPlayer02.ucPoints = 0;
                xPlayer02.ucWins++;

                if(xPlayer02.ucWins == systemMINIMUM_OF_VICTORIES) {
                    eCurrentGamePhase = eEndPhase;
                    xSemaphoreGive(xSemaphorePhase);
                }
            }
            sprintf(xPlayer02.pcPointsString, "%02d", xPlayer02.ucPoints);
            break;
        default:
            break;
        }
        portEXIT_CRITICAL();
    }
}

void vUpdatePlayerPositionTask(void *pvParameters) {
    QueuePlayerItem_t xQueuePlayerItem;

    vTaskSuspend(NULL);

    while(1) {
        xQueueReceive(xQueuePlayer, (void *) &xQueuePlayerItem, portMAX_DELAY);

        portENTER_CRITICAL();
        switch(xQueuePlayerItem.ePlayer) {
            case ePlayer1:
                vSystemUpdatePlayerPosition(&xPlayer01, xQueuePlayerItem.eMovement);
                break;
            case ePlayer2:
                vSystemUpdatePlayerPosition(&xPlayer02, xQueuePlayerItem.eMovement);
                break;
            default:
                break;
        }
        portEXIT_CRITICAL();
    }
}

/* Timers Callbacks */

void vRenderTimerCallback(TimerHandle_t xTimer) {
    xSemaphoreGive(xSemaphoreDisplay);
}

void vSoftwareDebouncingTimerCallback(TimerHandle_t xTimer) {
    portENABLE_INTERRUPTS();
}

/* Private Interrupt Handlers */

BaseType_t prvButtonInterruptHandler(Button eButton) {
    PlayerPad_t *pxPlayerPad = NULL;

    if(eButton == eGreenButton || eButton == eBlueButton) {
        pxPlayerPad = &xPlayerPad1;
    }
    else {
        pxPlayerPad = &xPlayerPad2;
    }

    UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();

    pxPlayerPad->eButton = eButton;

#ifdef systemSIMULATOR_EXECUTION
    pxPlayerPad->eEvent = !eSystemCheckButtonEvent(eButton); 
#else
    pxPlayerPad->eEvent = eSystemCheckButtonEvent(eButton); 
#endif

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(eCurrentGamePhase == ePlayPhase) {
        xSemaphoreGiveFromISR(xSemaphoreButton, &xHigherPriorityTaskWoken);
    }
    else if(eCurrentGamePhase != ePlayPhase && pxPlayerPad->eEvent == ePressed) {
        xSemaphoreGiveFromISR(xSemaphorePhase, &xHigherPriorityTaskWoken);
    }

    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

    return xHigherPriorityTaskWoken;
}

/* Global Interrupt Handler */

void vInterruptHandler(void) {
    uint32_t ulInterruptId = ulSystemBeginInterruptHandling();

    Button eButton = eSystemCheckInterruptSource(ulInterruptId);

    if(eButton == eInvalidButton) {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = prvButtonInterruptHandler(eButton);

    vSystemEndInterruptHandling(ulInterruptId);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Application Hooks */

void vApplicationMallocFailedHook(void) {
    portDISABLE_INTERRUPTS();
    vSystemSetLed(eRedLed);
	while(1);
}

/* Assert Handler */

void vAssertCalled(const char *pcFile, uint32_t ulLine) {
	portDISABLE_INTERRUPTS();
    vSystemSetLed(eRedLed);
	while(1); 
}

/* Private Configuration Functions */

static inline void prvSystemConfig(void) {
    eCurrentGamePhase = eInitalPhase;

    vSystemInit();

    vSystemSetLed(eGreenLed);
}

static inline void prvKernelConfig(void) {
    xQueueScore = xQueueCreate(2, sizeof(QueueScoreItem_t));
    xQueuePlayer = xQueueCreate(2, sizeof(QueuePlayerItem_t));

    xSemaphorePhase = xSemaphoreCreateBinary();
    xSemaphoreButton = xSemaphoreCreateBinary();
    xSemaphoreDisplay = xSemaphoreCreateBinary();

    xTimerDisplayRender = xTimerCreate(
            "TimerDisplayRender", 
            pdMS_TO_TICKS(systemDISPLAY_FRAME_DURATION_MS), 
            pdTRUE, 
            NULL,
            vRenderTimerCallback
    );
    xTimerSoftwareDebouncing = xTimerCreate(
            "TimerSoftwareDebouncing", 
            pdMS_TO_TICKS(systemDEBOUNCING_TIME_MS), 
            pdFALSE, 
            NULL,
            vSoftwareDebouncingTimerCallback
    );

    xTaskCreate(
            vGamePhaseHandlingTask,
            "TaskPhase",
            configMINIMAL_STACK_SIZE,
            NULL,
            3,
            &xGamePhaseHandlingTaskHandle
    );
    xTaskCreate(
            vInputHandlingTask,
            "TaskButton",
            configMINIMAL_STACK_SIZE,
            NULL,
            2,
            &xInputHandlingTaskHandle
    );
    xTaskCreate(vUpdateScoreTask,
            "TaskScore",
            2*configMINIMAL_STACK_SIZE,
            NULL,
            2,
            &xUpdateScoreTaskHandle
    );
    xTaskCreate(vUpdatePlayerPositionTask,
            "TaskPlayer",
            configMINIMAL_STACK_SIZE,
            NULL,
            1,
            &xUpdatePlayerPositionTaskHandle
    );
    xTaskCreate(vUpdateDisplayTask,
            "TaskDisplay",
            configMINIMAL_STACK_SIZE,
            NULL,
            3,
            &xUpdateDisplayTaskHandle
    );
}

static inline void prvKernelStart(void) {
    portENABLE_INTERRUPTS();
    vTaskStartScheduler();
}

static inline void prvKernelResume(void) {
    xTimerStart(xTimerDisplayRender, 0);
    vTaskResume(xUpdateDisplayTaskHandle);
    vTaskResume(xInputHandlingTaskHandle);
    vTaskResume(xUpdateScoreTaskHandle);
    vTaskResume(xUpdatePlayerPositionTaskHandle);
}

static inline void prvKernelPause(void) {
    xTimerStop(xTimerDisplayRender, 0);
    vTaskSuspend(xUpdateDisplayTaskHandle);
    vTaskSuspend(xInputHandlingTaskHandle);
    vTaskSuspend(xUpdateScoreTaskHandle);
    vTaskSuspend(xUpdatePlayerPositionTaskHandle);
}
