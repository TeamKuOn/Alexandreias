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
SemaphoreHandle_t xCanTxSemaphore = xSemaphoreCreateMutex();
SemaphoreHandle_t xCanPrintSemaphore = xSemaphoreCreateMutex();

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

void TaskCANSend(void *pvParameters) {

    for(;;) {

        byte data1[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
        byte data2[6] = {0x00, 0x03, 0x02, 0x04, 0x05, 0x07};

        if(xSemaphoreTake(xCanTxSemaphore, (TickType_t)1) == pdTRUE) {
            canSendStatus1 = CAN0.sendMsgBuf(CAN_SEND_ID_1, 0, can_send_dlc_1, data1);
            canSendStatus2 = CAN0.sendMsgBuf(CAN_SEND_ID_2, 0, can_send_dlc_2, data2);

            xSemaphoreGive(xCanTxSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskDisplayCANSendRes(void *pvParameters) {

    for(;;) {
        if(xSemaphoreTake(xCanPrintSemaphore, (TickType_t)1) == pdTRUE) {

            if (canSendStatus1 == 0) {
                Serial.println("Send Msg1 OK!");
            } else if (canSendStatus1 == 6) {
                Serial.println("Get TX buff time out... (CAN_GETTXBFTIMEOUT)");
            } else if (canSendStatus1 == 7) {
                Serial.println("Send Msg1 time out... (CAN_SENDMSGTIMEOUT)");
            }
            
            if (canSendStatus2 == 0) {
                Serial.println("Send Msg2 OK!");
            } else if (canSendStatus2 == 6) {
                Serial.println("Get TX buff time out... (CAN_GETTXBFTIMEOUT)");
            } else if (canSendStatus2 == 7) {
                Serial.println("Send Msg2 time out... (CAN_SENDMSGTIMEOUT)");
            }
        
            xSemaphoreGive(xCanPrintSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
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
    xTaskCreateUniversal(TaskCANSend, "CANSend", 2048, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskDisplayCANSendRes, "DisplayCANSendRes", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#elif defined(ATMEGA2560)
    xTaskCreate(TaskCANSend, "CANSend", 1024, NULL, PRIORITY_1, NULL);
    xTaskCreate(TaskDisplayCANSendRes, "DisplayCANSendRes", 1024, NULL, PRIORITY_0, NULL);
#endif

}

void loop() {}
