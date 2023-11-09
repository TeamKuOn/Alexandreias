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

#define DARKER_GREY 0x18E3
#define ROTATE_SIDE 2

#define LOOP_DELAY 0

uint32_t runTime = 0;       // time for next update

int reading = 0; // Value to be displayed
int d = 0; // Variable used for the sinewave test waveform
bool range_error = 0;
int8_t ramp = 1;

static int16_t xpos = tft.width() / 2;
static int16_t ypos = tft.height() / 2;
static uint8_t radius = xpos;
uint16_t tmp = 0;

int dummy_val = 0;
int dummy_val_angle = 0;

bool initMeter = true;

void ringMeter(int x, int y, int r, int val, const char *units){
    static uint16_t last_angle = 30;
    char value[10];

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    sprintf(value, "%d", val);
    tft.drawString(value, x - 20, y - 10, 4);
    tft.drawString(units, x + 20, y + 10, 4);

    if(initMeter){
        initMeter = false;
        last_angle = 30;
        tft.fillCircle(x, y, r, DARKER_GREY);
        // tft.drawSmoothCircle(x, y, r, TFT_SILVER, DARKER_GREY);
        uint16_t tmp = r - 3;
        tft.drawArc(x, y, tmp, tmp - tmp / 5, last_angle, 330, TFT_BLACK, DARKER_GREY);
    }

    r -= 3;

    // Range here is 0-100 so value is scaled to an angle 30-330
    int val_angle = map(val, 0, 100, 30, 330);


    if (last_angle != val_angle) {
        // Allocate a value to the arc thickness dependant of radius
        uint8_t thickness = r / 5;
        if ( r < 25 ) thickness = r / 3;

        // Update the arc, only the zone between last_angle and new val_angle is updated
        if (val_angle > last_angle) {
        tft.drawArc(x, y, r, r - thickness, last_angle, val_angle, TFT_SKYBLUE, TFT_BLACK); // TFT_SKYBLUE random(0x10000)
        }
        else {
        tft.drawArc(x, y, r, r - thickness, val_angle, last_angle, TFT_BLACK, DARKER_GREY);
        }
        last_angle = val_angle; // Store meter arc position for next redraw
    }
}

void TaskDataGenerator(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
            dummy_val = int(random(40, 80));

            xSemaphoreGive(xSemaphore1);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void TaskDisplayData(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            ringMeter(xpos, ypos, radius, dummy_val, "W"); // Draw analogue meter

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

    /* Random seed */
    randomSeed(millis());

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskDisplayData, "TaskDisplayData", 2048, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskDataGenerator, "TaskDataGenerator", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif

}

void loop(){}
