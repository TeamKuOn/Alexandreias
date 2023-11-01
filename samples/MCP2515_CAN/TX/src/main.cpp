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

SemaphoreHandle_t xCanSemaphore;

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
        if(xSemaphoreTake(xCanSemaphore, (TickType_t)1) == pdTRUE) {

            byte data1[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
            byte data2[6] = {0x00, 0x03, 0x02, 0x04, 0x05, 0x07};

            canSendStatus1 = CAN0.sendMsgBuf(CAN_SEND_ID_1, 0, can_send_dlc_1, data1);
            canSendStatus2 = CAN0.sendMsgBuf(CAN_SEND_ID_2, 0, can_send_dlc_2, data2);

            xSemaphoreGive(xCanSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskDisplayCANSendRes(void *pvParameters) {

    for(;;) {

        if (canSendStatus1 == 0) {
            Serial.println("Send Msg1 OK!");
        } else if (canSendStatus1 == 6) {
            Serial.println("Get TX buff time out... (CAN_GETTXBFTIMEOUT)");
        } else if (canSendStatus1 == 7) {
            Serial.println("Send Msg1 time out... (CAN_SENDMSGTIMEOUT)");
        }
        
        if (canSendStatus2 == 0) {
            Serial.println("Send Msg1 OK!");
        } else if (canSendStatus2 == 6) {
            Serial.println("Get TX buff time out... (CAN_GETTXBFTIMEOUT)");
        } else if (canSendStatus2 == 7) {
            Serial.println("Send Msg1 time out... (CAN_SENDMSGTIMEOUT)");
        }
        

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void setup() {
    Serial.begin(115200);

    /* CAN Setting up */
    Serial.println("CAN Starting up");

    if(CAN0.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
    else Serial.println("Error Initializing MCP2515...");

    CAN0.setMode(MCP_NORMAL);
    pinMode(CAN0_INT, INPUT);
    Serial.println("CAN Setup complete");

    /* Semaphore setting */
    if((xCanSemaphore = xSemaphoreCreateMutex()) != NULL) {
        xSemaphoreGive((xCanSemaphore));
    }

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskCANSend, "CANSend", 1024, NULL, 1, NULL, 1);
    xTaskCreateUniversal(TaskDisplayCANSendRes, "DisplayCANSendRes", 1024, NULL, 0, NULL, 0);
#elif defined(ATMEGA2560)
    xTaskCreate(TaskCANSend, "CANSend", 1024, NULL, 1, NULL);
    xTaskCreate(TaskDisplayCANSendRes, "DisplayCANSendRes", 1024, NULL, 0, NULL);
#endif

}

void loop() {}
