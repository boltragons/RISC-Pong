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
            
/* Synchronization Handles */

QueueHandle_t xQueuePlayer;

QueueHandle_t xQueueScore;

SemaphoreHandle_t xSemaphoreButton;

SemaphoreHandle_t xSemaphoreDisplay;
            
/* Communication Handles */

PlayerPad_t xPlayerPad1;

PlayerPad_t xPlayerPad2;

Player_t xPlayer01;

Player_t xPlayer02;

Ball_t xBall;
            
/* Tasks Procedures */
            
void vUpdateDisplay(void *pvParameters);
            
void vInputHandlingTask(void *pvParameters);
            
void vUpdateScoreTask(void *pvParameters);
            
void vUpdatePlayerPositionTask(void *pvParameters);

/* Timers Callbacks */

void vRenderTimerCallback(TimerHandle_t xTimer);

/* Private Interrupt Handlers */

BaseType_t prvButtonInterruptHandler(Button eButton);

/* Private Configuration Functions */

static inline void prvSystemConfig(void);

static inline void prvKernelConfig(void);

static inline void prvKernelStart(void);

/* Main Code */

int main(void) {
    prvSystemConfig();

    prvKernelConfig();

    prvKernelStart();

    while(1);
}       
                  
/* Tasks Procecudures */
        
void vUpdateDisplay(void *pvParameters) {
    QueueScoreItem_t xQueueScoreItem;

    while(1) {
        xSemaphoreTake(xSemaphoreDisplay, portMAX_DELAY);

        portENTER_CRITICAL();
        vSystemDisplayUpdateFrame(&xPlayer01, &xPlayer02, &xBall);
        xQueueScoreItem.eWinner = vSystemUpdateBallPosition(&xBall, &xPlayer01, &xPlayer02);

        if(xQueueScoreItem.eWinner != eNonePlayer) {
            vSystemGetBallDefaultConfig(&xBall);
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

    while(1) {
        xQueueReceive(xQueueScore, (void *) &QueueScoreItem, portMAX_DELAY);

        portENTER_CRITICAL();
        switch(QueueScoreItem.eWinner) {
        case ePlayer1:
            xPlayer01.ulPoints++;
            sprintf(xPlayer01.pcPointsString, "%03ld", xPlayer01.ulPoints);
            break;
        case ePlayer2:
            xPlayer02.ulPoints++;
            sprintf(xPlayer02.pcPointsString, "%03ld", xPlayer02.ulPoints);
            break;
        default:
            break;
        }
        portEXIT_CRITICAL();
    }
}

void vUpdatePlayerPositionTask(void *pvParameters) {
    QueuePlayerItem_t xQueuePlayerItem;

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

    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xSemaphoreButton, &xHigherPriorityTaskWoken);

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
    vSystemGetPlayerDefaultConfig(&xPlayer01, ePlayer1);
    vSystemGetPlayerDefaultConfig(&xPlayer02, ePlayer2);
    vSystemGetBallDefaultConfig(&xBall);

    vSystemInit();

    vSystemSetLed(eGreenLed);

    vSystemDisplayMainScreen();

    while(!eSystemReadButton(eGreenButton) 
        && !eSystemReadButton(eBlueButton) 
        && !eSystemReadButton(eYellowButton) 
        && !eSystemReadButton(eRedButton));
}

static inline void prvKernelConfig(void) {
    xQueueScore = xQueueCreate(2, sizeof(QueueScoreItem_t));
    xQueuePlayer = xQueueCreate(2, sizeof(QueuePlayerItem_t));

    xSemaphoreButton = xSemaphoreCreateBinary();
    xSemaphoreDisplay = xSemaphoreCreateBinary();

    TimerHandle_t xTimerDisplayRender = xTimerCreate("TimerDisplayRender", pdMS_TO_TICKS(systemDISPLAY_FRAME_DURATION_MS), pdTRUE, NULL, vRenderTimerCallback);
    xTimerStart(xTimerDisplayRender, 0);

    xTaskCreate(vInputHandlingTask, "TaskButton", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vUpdateScoreTask, "TaskScore", 2*configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vUpdatePlayerPositionTask, "TaskPlayer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vUpdateDisplay, "TaskDisplay", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
}

static inline void prvKernelStart(void) {
    portENABLE_INTERRUPTS();
    vTaskStartScheduler();
}
