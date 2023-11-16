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

MCP_CAN CAN0(CHIP_SELECT);

#define STD_FRAME 0
#define EXT_FRAME 0
#define MAX_DATA_LEN 8

struct can_frame {
    unsigned long can_id;
    byte can_dlc;
    byte data[MAX_DATA_LEN];
};

struct can_frame canMsg0;
struct can_frame canMsg1;
struct can_frame canMsg2;

#define CAN_SEND_ID_0 0x0F5     // 0x0F5 = 245
#define CAN_SEND_ID_1 0x0F6     // 0x0F6 = 246
#define CAN_SEND_ID_2 0x0F7     // 0x0F7 = 247

byte canSendStatus0;
byte canSendStatus1;
byte canSendStatus2;



void encode_double2byte(double d, byte *byteArr) {
    byte* bytes = (byte*) &d;
    for(int i = 0; i < sizeof(double); i++) {
        byteArr[i] = bytes[i];
        if(i > MAX_DATA_LEN) {
            break;
        }
    }
}

void makeDoubleCanMsg(struct can_frame *canMsg, unsigned long can_id, double f) {
    canMsg->can_id = can_id;
    encode_double2byte(f, canMsg->data);
    canMsg->can_dlc = sizeof(canMsg->data);
}


void encode_float2byte(float f, byte *byteArr) {
    byte* bytes = (byte*) &f;
    for(int i = 0; i < sizeof(float); i++) {
        byteArr[i] = bytes[i];
        if(i > MAX_DATA_LEN) {
            break;
        }
    }
}

void makeFloatCanMsg(struct can_frame *canMsg, unsigned long can_id, float f) {
    canMsg->can_id = can_id;
    encode_float2byte(f, canMsg->data);
    canMsg->can_dlc = sizeof(canMsg->data);
}

void encode_int2byte(int i, byte *byteArr) {
    byte* bytes = (byte*) &i;
    for(int i = 0; i < sizeof(int); i++) {
        byteArr[i] = bytes[i];
        if(i > MAX_DATA_LEN) {
            break;
        }
    }
}

void makeIntCanMsg(struct can_frame *canMsg, unsigned long can_id, int i) {
    canMsg->can_id = can_id;
    encode_int2byte(i, canMsg->data);
    canMsg->can_dlc = sizeof(canMsg->data);
}

void TaskCANSend(void *pvParameters) {
    int val_0 = 123;
    float val_1 = 33.6673188;
    double val_2 = 135.3545314;

    for(;;) {

        makeIntCanMsg(&canMsg0, CAN_SEND_ID_0, val_0);
        makeFloatCanMsg(&canMsg1, CAN_SEND_ID_1, val_1);
        makeDoubleCanMsg(&canMsg2, CAN_SEND_ID_2, val_2);

        if(xSemaphoreTake(xCanTxSemaphore, (TickType_t)1) == pdTRUE) {
            canSendStatus0 = CAN0.sendMsgBuf(canMsg0.can_id, STD_FRAME, canMsg0.can_dlc, canMsg0.data);
            canSendStatus1 = CAN0.sendMsgBuf(canMsg1.can_id, STD_FRAME, canMsg1.can_dlc, canMsg1.data);
            canSendStatus2 = CAN0.sendMsgBuf(canMsg2.can_id, STD_FRAME, canMsg2.can_dlc, canMsg2.data);

            xSemaphoreGive(xCanTxSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskDisplayCANSendRes(void *pvParameters) {

    for(;;) {
        if(xSemaphoreTake(xCanPrintSemaphore, (TickType_t)1) == pdTRUE) {
            if(canSendStatus0 == 0) {
                Serial.println("Send Msg0 OK!");
            } else if (canSendStatus0 == 6) {
                Serial.println("Get TX buff time out... (CAN_GETTXBFTIMEOUT)");
            } else if (canSendStatus0 == 7) {
                Serial.println("Send Msg0 time out... (CAN_SENDMSGTIMEOUT)");
            }

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
