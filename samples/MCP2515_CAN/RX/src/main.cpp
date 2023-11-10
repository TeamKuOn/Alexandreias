#define ESP32_DEVKIT
// #define ATMEGA2560

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

#define CAN_SEND_ID_1 0x0F6
#define CAN_SEND_ID_2 0x0F7

MCP_CAN CAN0(CHIP_SELECT);

int can_send_dlc_1 = 8;
int can_send_dlc_2 = 6;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];

int receiveRes;
byte canSendStatus1;
byte canSendStatus2;

void IRAM_ATTR MCP2515_Read_ISR() {
    portENTER_CRITICAL_ISR(&canRxMutex);

    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    portEXIT_CRITICAL_ISR(&canRxMutex);
}

void TaskDisplayCANReceiveRes(void *pvParameters) {

    for(;;) {

        if(xSemaphoreTake(xCanSemaphore, (TickType_t)1) == pdTRUE) {

            if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
                sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
            else
                sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
        
            Serial.print(msgString);

            if((rxId & 0x40000000) == 0x40000000){    
                sprintf(msgString, " REMOTE REQUEST FRAME");
                Serial.print(msgString);
            } else {
                for(byte i = 0; i<len; i++){
                    sprintf(msgString, " 0x%.2X", rxBuf[i]);
                    Serial.print(msgString);
                }
            }
            
            Serial.println();

            xSemaphoreGive(xCanSemaphore);
        }
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
