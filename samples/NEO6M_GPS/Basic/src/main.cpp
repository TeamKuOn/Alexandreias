#define ESP32_DEVKIT

/* Main library */
#include "Arduino.h"

/* Communication library */

/* Device library */
#include <TinyGPS++.h>

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t xSemaphore1 = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphore2 = xSemaphoreCreateMutex();
portMUX_TYPE Mutex = portMUX_INITIALIZER_UNLOCKED;

/* GPS settings */
TinyGPSPlus gps;
#define NEO6M Serial2
#define GPS_BAUD 9600
unsigned int prev_sec = 0;

struct GPS_DATA {
    double latitude;
    double longitude;

    unsigned int date;
    unsigned int time;

    double speed;
    double course;

    double altitude;

    unsigned int hdop;
};

struct GPS_DATA gps_data;


void TaskGetGpsSignal(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
            if (NEO6M.available()) {
                gps.encode(NEO6M.read());
                prev_sec = gps.time.second();
            }

            xSemaphoreGive(xSemaphore1);
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void TaskPrintData(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            if(prev_sec != gps.time.second()) {
                gps_data.latitude = gps.location.lat();
                gps_data.longitude = gps.location.lng();

                gps_data.date = gps.date.value();
                gps_data.time = gps.time.value();

                gps_data.speed = gps.speed.kmph();
                gps_data.course = gps.course.deg();

                gps_data.altitude = gps.altitude.meters();

                gps_data.hdop = gps.hdop.value();

                Serial.print("Latitude: ");
                Serial.println(gps_data.latitude, 6);
                Serial.print("Longitude: ");
                Serial.println(gps_data.longitude, 6);

                Serial.print("Date: ");
                Serial.println(gps_data.date);
                Serial.print("Time: ");
                Serial.println(gps_data.time);

                Serial.print("Speed: ");
                Serial.println(gps_data.speed);
                Serial.print("Course: ");
                Serial.println(gps_data.course);

                Serial.print("Altitude: ");
                Serial.println(gps_data.altitude);

                Serial.print("HDOP: ");
                Serial.println(gps_data.hdop);

                prev_sec = gps.time.second();
            }

            xSemaphoreGive(xSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(400));
    }
}


void setup(){
    /* Serial setting */
    Serial.begin(115200);
    NEO6M.begin(GPS_BAUD);

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskGetGpsSignal, "TaskGetGpsSignal", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskPrintData, "TaskPrintData", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif

}

void loop(){}


