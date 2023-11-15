#define ESP32_DEVKIT
// #define ATMEGA2560

#define RX_RAW_DATA
// #define RX_DECODED_DATA

/* Main library */
#include "Arduino.h"

#if defined(ATMEGA2560)
#include "Arduino_FreeRTOS.h"
#include "semphr.h"
#endif

/* Communication library */
#include <SPI.h>

/* Device library */
#include <mcp_can.h>

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t xCanSemaphore = xSemaphoreCreateMutex();
portMUX_TYPE canRxMutex = portMUX_INITIALIZER_UNLOCKED;

// typedef unsigned char ___u64Byte;

/* CAN setting */
#if defined(ESP32_DEVKIT)
#define CHIP_SELECT 5
#define CAN0_INT 21
#elif defined(ATMEGA2560)
#define CHIP_SELECT 53
#define CAN0_INT 2
#endif

MCP_CAN CAN0(CHIP_SELECT);

struct can_frame {
    unsigned long can_id;
    byte can_dlc;
    byte data[8];
};

struct can_frame canMsg;

char msgString[128];


void IRAM_ATTR MCP2515_Read_ISR() {
    portENTER_CRITICAL_ISR(&canRxMutex);

    CAN0.readMsgBuf(&canMsg.can_id, &canMsg.can_dlc, canMsg.data);

    portEXIT_CRITICAL_ISR(&canRxMutex);
}

void TaskDisplayCANReceiveRes(void *pvParameters) {

    for(;;) {

#if defined(RX_RAW_DATA)
        if(xSemaphoreTake(xCanSemaphore, (TickType_t)10) == pdTRUE) {

            if((canMsg.can_id & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
                sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (canMsg.can_id & 0x1FFFFFFF), canMsg.can_dlc);
            else
                sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", canMsg.can_id, canMsg.can_dlc);

            xSemaphoreGive(xCanSemaphore);
        }
        
        Serial.print(msgString);

        if(xSemaphoreTake(xCanSemaphore, (TickType_t)10) == pdTRUE) {

            if((canMsg.can_id & 0x40000000) == 0x40000000){    
                sprintf(msgString, " REMOTE REQUEST FRAME");
                Serial.print(msgString);
            } else {
                for(byte i = 0; i<canMsg.can_dlc; i++){
                    sprintf(msgString, " 0x%.2X", canMsg.data[i]);
                Serial.print(msgString);
}
            }
            
            xSemaphoreGive(xCanSemaphore);
        }

        Serial.println();
#elif defined(RX_DECODED_DATA)
#endif

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void setup() {
    Serial.begin(115200);

    /* CAN Setting up */
    Serial.println("CAN Starting up");

    while(CAN0.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ) != CAN_OK){
        Serial.println("Error Initializing MCP2515...");
    }
    Serial.println("MCP2515 Initialized Successfully!");

    CAN0.setMode(MCP_NORMAL);
    pinMode(CAN0_INT, INPUT);
    Serial.println("CAN Setup complete");

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskDisplayCANReceiveRes, "DisplayCANReceiveRes", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#elif defined(ATMEGA2560)
    xTaskCreate(TaskDisplayCANReceiveRes, "DisplayCANReceiveRes", 1024, NULL, PRIORITY_0, NULL);
#endif

    /* Interrupt setting */
    attachInterrupt(digitalPinToInterrupt(CAN0_INT), MCP2515_Read_ISR, FALLING);
}

void loop() {}
