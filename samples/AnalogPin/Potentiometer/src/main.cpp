#define ESP32_DEVKIT
// #define ATMEGA2560

/* Main library */
#include "Arduino.h"

#if defined(ATMEGA2560)
#include "Arduino_FreeRTOS.h"
#include "semphr.h"
#endif

/* Communication library */

/* Device library */

/* Semaphore settin */
SemaphoreHandle_t xAnalogpinSemaphore;

#if defined(ESP32_DEVKIT)
#define POTENTIOMETER_PIN 34
#define ANALOG_MAX 4096
#elif defined(ATMEGA2560)
#define POTENTIOMETER_PIN A0
#endif

void TaskAnalogRead(void *pvParameters) {

    int analog_val = 0;
    int mV_val = 0;

    for (;;) {
        if (xSemaphoreTake(xAnalogpinSemaphore, (TickType_t)10) == pdTRUE) {
            analog_val = analogRead(POTENTIOMETER_PIN);
            mV_val = analogReadMilliVolts(POTENTIOMETER_PIN);

            Serial.println(analog_val);
            Serial.print(mV_val); Serial.println("[mV]");

            xSemaphoreGive(xAnalogpinSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void setup() {
    /* Serial setting */
    Serial.begin(112500);

    /*Semaphore setting*/
    if ((xAnalogpinSemaphore = xSemaphoreCreateMutex()) != NULL) {
        xSemaphoreGive((xAnalogpinSemaphore));
    }

    /* Pin setting */
    pinMode(POTENTIOMETER_PIN, ANALOG);
#if defined(ESP32_DEVKIT)
    analogSetPinAttenuation(POTENTIOMETER_PIN, ADC_11db);
#endif

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskAnalogRead, "TaskAnalogRead", 128, NULL, 1, NULL, 1);
#elif defined(ATMEGA2560)
    xTaskCreate(TaskAnalogRead, "TaskAnalogRead", 128, NULL, 1, NULL);
#endif

}

void loop() {}
