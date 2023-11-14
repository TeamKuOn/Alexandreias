#define ESP32_DEVKIT

/* Main library */
#include "Arduino.h"

/* Communication library */

/* Device library */

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t xSemaphore1 = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphore2 = xSemaphoreCreateMutex();

/* Voltage Transmitter setting */
#if defined(ESP32_DEVKIT)
#define ANALOG_PIN 34
#define ANALOG_MAX 4096
#endif
#define MAX_VOLT 3.3    // 0 to 3.3 [volt]
#define TRNS_INPUT_UPPER 200  // 0 to 200 [volt]

double origin_volt = 0.0; 
double estimate_volt = 0.0;


void TaskSample1(void *pvParameters){

    for(;;) {
        origin_volt = analogRead(ANALOG_PIN);
        if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
            estimate_volt = map(origin_volt, 0, ANALOG_MAX, 0, TRNS_INPUT_UPPER);

            xSemaphoreGive(xSemaphore1);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskSample2(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            Serial.println(estimate_volt);

            xSemaphoreGive(xSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void setup(){
    /* Serial setting */
    Serial.begin(115200);

    /* Pin setting */
    pinMode(ANALOG_PIN, ANALOG);
#if defined(ESP32_DEVKIT)
    analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
#endif

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskSample1, "TaskSample1", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskSample2, "TaskSample2", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif

}

void loop(){}


