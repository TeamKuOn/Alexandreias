#define ESP32_DEVKIT

/* Main library */
#include "Arduino.h"

/* Communication library */
#include <SPI.h>

/* Device library */
#if defined(ESP32_DEVKIT)
#include <TFT_eSPI.h>
#endif

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t xSemaphore1 = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphore2 = xSemaphoreCreateMutex();
portMUX_TYPE Mutex = portMUX_INITIALIZER_UNLOCKED;

/* GC9A01 setting*/
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define ROTATE_SIDE 2

static int16_t xpos = tft.width() / 4;
static int16_t ypos = tft.height() / 4;
static uint8_t radius = xpos;

// Font setting
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#define SPR_W 120
#define SPR_H 160

uint16_t  spr_width = 0;

unsigned int dummy_val = 0;



void TaskDisplayData(void *pvParameters){
    char value[10];

    for(;;) {
        if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
            sprintf(value, "%d", dummy_val);

            xSemaphoreGive(xSemaphore1);
        }

        spr.fillSprite(TFT_GREEN);

        // spr.setTextFont(1);
        spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
        spr.setTextSize(3);
        spr.setTextDatum(MC_DATUM);
        spr.drawString(value, 40, 40);

        spr.pushSprite(xpos, ypos);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void TaskDataGenerator(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            dummy_val = int(random(40, 80));

            xSemaphoreGive(xSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void setup(){
    /* Serial setting */
    Serial.begin(115200);

    /* GC9A01 setting */
    tft.begin();
    tft.setRotation(ROTATE_SIDE);
    tft.fillScreen(TFT_BLACK);

    spr.createSprite(SPR_W, SPR_H);
    spr.loadFont(NotoSansBold36);

    /* Random seed */
    randomSeed(millis());

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskDisplayData, "TaskDisplayData", 4096, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskDataGenerator, "TaskDataGenerator", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif

}

void loop(){}
