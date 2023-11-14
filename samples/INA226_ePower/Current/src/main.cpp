#define ESP32_DEVKIT

/* Main library */
#include "Arduino.h"

/* Communication library */
#include <Wire.h>

/* Device library */
#include <INA226_asukiaaa.h>

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t xSemaphore1 = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphore2 = xSemaphoreCreateMutex();

/* INA226 setting */
#if defined(ESP32_DEVKIT)
#define WIRE_SDA 21
#define WIRE_SCL 22
#endif
#define SHUNT_RESISTOR 2 // mill Ohm

const uint16_t ina226calib = INA226_asukiaaa::calcCalibByResistorMilliOhm(SHUNT_RESISTOR);
INA226_asukiaaa voltCurrMeter(INA226_ASUKIAAA_ADDR_A0_GND_A1_GND, ina226calib);

struct INA226Data {
    int16_t mill_amp;
    int16_t mill_volt;
    int16_t mill_watt;
};

struct INA226Data ina226;



void TaskSample1(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
            voltCurrMeter.readMA(&ina226.mill_amp);
            voltCurrMeter.readMV(&ina226.mill_volt);
            voltCurrMeter.readMW(&ina226.mill_watt);

            xSemaphoreGive(xSemaphore1);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskSample2(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            Serial.println(String(ina226.mill_amp) + "mA");
            Serial.println(String(ina226.mill_volt) + "mV");
            Serial.println(String(ina226.mill_watt) + "mW");

            xSemaphoreGive(xSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void setup(){
    /* Serial setting */
    Serial.begin(115200);
#if defined(ESP32_DEVKIT)
    Wire.begin(WIRE_SDA, WIRE_SCL);
    voltCurrMeter.setWire(&Wire);
#endif

    /* INA226 setting */
    while(voltCurrMeter.begin() != 0) {
        Serial.println("Failed to begin INA226");
    }

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskSample1, "TaskSample1", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskSample2, "TaskSample2", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif
}

void loop(){}

