#include <stdint.h> 
#include <stdlib.h> 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "system.h"

/* Useful Macros */

#define assert(x) configASSERT(x)
            
/* Communication Handles Types */

typedef enum LedCommand {
    eSet,
    eClear,
    eToggle
} LedCommand;

typedef struct QueueLedItem_t {
    Led eLed;
    LedCommand eCommand;
} QueueLedItem_t;

typedef struct QueuePlayerItem_t {
    PlayerId ePlayer;
    PlayerMovement eMovement;
} QueuePlayerItem_t;

typedef struct PlayerPad_t {
    Button eButton;
    ButtonEvent eEvent;
} PlayerPad_t;
            
/* Synchronization Handles */

QueueHandle_t xQueueLed;

QueueHandle_t xQueuePlayer;

SemaphoreHandle_t xSemaphoreButton;
            
/* Communication Handles */

PlayerPad_t xPlayerPad1;

PlayerPad_t xPlayerPad2;
            
/* Tasks Procecudures */

void vLedTask(void *pvParameters);
            
void vButtonTask(void *pvParameters);
            
void vDisplayPlayerTask(void *pvParameters);
            
void vDisplayBallTask(void *pvParameters);

/* Private Interrupt Handlers */

BaseType_t prvButtonInterruptHandler(Button eButton);

/* Main Code */

int main(void) {
    vSystemInit();

    vSystemSetLed(eGreenLed);

    xQueueLed = xQueueCreate(1, sizeof(QueueLedItem_t));
    xQueuePlayer = xQueueCreate(2, sizeof(QueuePlayerItem_t));

    xSemaphoreButton = xSemaphoreCreateBinary();

    xTaskCreate(vLedTask, "TaskLed", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vButtonTask, "TaskButton", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vDisplayPlayerTask, "TaskPlayer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    // xTaskCreate(vDisplayBallTask, "TaskBall", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    portENABLE_INTERRUPTS();

    vTaskStartScheduler();
    
    while(1);
}       
                  
/* Tasks Procecudures */

void vLedTask(void *pvParameters) {
    TickType_t xLastWakeTick = xTaskGetTickCount();

    QueueLedItem_t xQueueLedItem;
    
    while(1) {
        xQueueReceive(xQueueLed, (void *) &xQueueLedItem, portMAX_DELAY);

        switch (xQueueLedItem.eCommand) {
        case eSet:
            vSystemSetLed(xQueueLedItem.eLed);
            break;
        case eClear:
            vSystemClearLed(xQueueLedItem.eLed);
            break;
        case eToggle:
            vSystemToggleLed(xQueueLedItem.eLed);
            break;
        }

        vTaskDelayUntil(&xLastWakeTick, pdMS_TO_TICKS(70));
    }
} 
            
void vButtonTask(void *pvParameters) {
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
            if(ulPad == ePlayer1) {
                vSystemSetLed(eBlueLed);
            }
            else {
                vSystemSetLed(eYellowLed);
            }

            portDISABLE_INTERRUPTS();
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
            portENABLE_INTERRUPTS();

            if(ulPad == ePlayer1) {
                vSystemClearLed(eBlueLed);
            }
            else {
                vSystemClearLed(eYellowLed);
            }
        }
    }
}  

void vDisplayPlayerTask(void *pvParameters) {
    TickType_t xLastWakeTick = xTaskGetTickCount();

    Player_t xPlayer01;
    Player_t xPlayer02;

    vSystemGetPlayerDefaultConfig(&xPlayer01, ePlayer1);
    vSystemGetPlayerDefaultConfig(&xPlayer02, ePlayer2);

    QueuePlayerItem_t xQueuePlayerItem;

    while(1) {
        vTaskDelayUntil(&xLastWakeTick, pdMS_TO_TICKS(16));

        vSystemDisplayDrawPlayer(&xPlayer01);
        vSystemDisplayDrawPlayer(&xPlayer02);

        xQueueReceive(xQueuePlayer, (void *) &xQueuePlayerItem, portMAX_DELAY);

        switch(xQueuePlayerItem.ePlayer) {
            case ePlayer1:
                vSystemUpdatePlayerPosition(&xPlayer01, xQueuePlayerItem.eMovement);
                break;
            case ePlayer2:
                vSystemUpdatePlayerPosition(&xPlayer02, xQueuePlayerItem.eMovement);
                break;
        }
    }
}
            
void vDisplayBallTask(void *pvParameters) {
    TickType_t xLastWakeTick = xTaskGetTickCount();

    Ball_t xBall;
    vSystemGetBallDefaultConfig(&xBall);

    while(1) {
        vSystemDisplayDrawBall(&xBall);
        vTaskDelayUntil(&xLastWakeTick, pdMS_TO_TICKS(20));
    }
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

    pxPlayerPad->eButton = eButton;

    pxPlayerPad->eEvent = !eSystemCheckButtonEvent(eButton); 

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xSemaphoreButton, &xHigherPriorityTaskWoken);

    return xHigherPriorityTaskWoken;
}

/* Global Interrupt Handler */

void vInterruptHandler(void) {
    portDISABLE_INTERRUPTS();

    uint32_t ulInterruptId = ulSystemBeginInterruptHandling();

    Button eButton = eSystemCheckInterruptSource(ulInterruptId);

    if(eButton == eInvalidButton) {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = prvButtonInterruptHandler(eButton);

    vSystemEndInterruptHandling(ulInterruptId);
    
    portENABLE_INTERRUPTS();

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
