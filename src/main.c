#include <stdint.h> 
#include <stdlib.h> 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "system.h"

typedef enum LedCommand {
    eSet,
    eClear,
    eToggle
} LedCommand;

typedef struct QueueLedItem_t {
    Led eLed;
    LedCommand eMovement;
} QueueLedItem_t;

typedef struct QueuePlayerItem_t {
    PlayerId ePlayer;
    PlayerMovement eMovement;
} QueuePlayerItem_t;

QueueHandle_t xQueueLed;

QueueHandle_t xQueuePlayer;
            
void vLedTask(void *pvParameters);
            
void vButtonTask(void *pvParameters);
            
void vDisplayPlayerTask(void *pvParameters);
            
void vDisplayBallTask(void *pvParameters);

int main(void) {
    vSystemInit();

    xQueueLed = xQueueCreate(1, sizeof(QueueLedItem_t));
    xQueuePlayer = xQueueCreate(1, sizeof(QueuePlayerItem_t));

    xTaskCreate(vLedTask, "TaskLed", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vButtonTask, "TaskButton", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vDisplayPlayerTask, "TaskPlayer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vDisplayBallTask, "TaskBall", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();
    
    while(1);
}       
            
void vLedTask(void *pvParameters) {
    TickType_t xLastWakeTick = xTaskGetTickCount();

    QueueLedItem_t xQueueLedItem;
    
    while(1) {
        xQueueReceive(xQueueLed, (void *) &xQueueLedItem, portMAX_DELAY);

        switch (xQueueLedItem.eMovement) {
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
    TickType_t xLastWakeTick = xTaskGetTickCount();

    QueuePlayerItem_t xQueuePlayerItem;

    while(1) {
        BaseType_t xNewUserInput = pdFALSE;

        if(eSystemReadButton(eGreenButton)) {
            xQueuePlayerItem.ePlayer = ePlayer1;
            xQueuePlayerItem.eMovement = eUp;
            xNewUserInput = pdTRUE;
        }
        else if(eSystemReadButton(eBlueButton)) {
            xQueuePlayerItem.ePlayer = ePlayer1;
            xQueuePlayerItem.eMovement = eDown;
            xNewUserInput = pdTRUE;
        }
        else if(eSystemReadButton(eYellowButton)) {
            xQueuePlayerItem.ePlayer = ePlayer2;
            xQueuePlayerItem.eMovement = eDown;
            xNewUserInput = pdTRUE;
        }
        else if(eSystemReadButton(eRedButton)) {
            xQueuePlayerItem.ePlayer = ePlayer2;
            xQueuePlayerItem.eMovement = eUp;
            xNewUserInput = pdTRUE;
        }

        if(xNewUserInput == pdTRUE) {
            xQueueSendToBack(xQueuePlayer, (void *) &xQueuePlayerItem, pdMS_TO_TICKS(10));
        }
        
        vTaskDelayUntil(&xLastWakeTick, pdMS_TO_TICKS(20));
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
        vTaskDelayUntil(&xLastWakeTick, pdMS_TO_TICKS(20));

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
