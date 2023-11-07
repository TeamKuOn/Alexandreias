#define ESP32_DEVKIT

/* Main library */
#include "Arduino.h"
#include "math.h"

/* Communication library */

/* Device library */

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_2 2
#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t accessSemaphore = xSemaphoreCreateMutex();
SemaphoreHandle_t accessSemaphore2 = xSemaphoreCreateMutex();
portMUX_TYPE pulseMutex = portMUX_INITIALIZER_UNLOCKED;

/* Pulse signal config */
unsigned int TIME_OUT_MICROS   = 1000000;     // count up time out [1 second]
unsigned int PULSE_P_ROUNDS    = 48;          // pulse per 1 round

volatile unsigned long previous_micros = 0;   // previous time [micro second]
volatile unsigned long current_micros = 0;    // current time [micro second]
volatile unsigned long pulse_interval_micros = 0;    // current time [micro second]

/* Global variable */
#if defined(ESP32_DEVKIT)
const int PULSE_INT_PIN = 18;
#endif

double WHEEL_DIAMETER = 35.56;
double WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER * PI;
double wheel_rotate_speed_mps = 0.0;
double wheel_rotate_speed_kmph = 0.0;



void IRAM_ATTR PULSE_SIGNAL_ISR() {
    portENTER_CRITICAL_ISR(&pulseMutex);

    current_micros = micros();
    pulse_interval_micros = current_micros - previous_micros;
    previous_micros = current_micros;

    portEXIT_CRITICAL_ISR(&pulseMutex);
}


void TaskPulseWatchdog(void *pvParameters){
    volatile unsigned long wd_current_micros = 0;    // current time [micro second]

    for(;;) {
        portENTER_CRITICAL_ISR(&pulseMutex);

        wd_current_micros = micros();
        if(wd_current_micros - previous_micros > TIME_OUT_MICROS) {
            pulse_interval_micros = TIME_OUT_MICROS;
        }

        portEXIT_CRITICAL_ISR(&pulseMutex);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void TaskSpeedCalc(void *pvParameters){
    volatile unsigned long psuedo_rotate_micros = 0;   // psuedo round time [10 micro second]

    for(;;) {
        portENTER_CRITICAL_ISR(&pulseMutex);
        psuedo_rotate_micros = pulse_interval_micros * PULSE_P_ROUNDS;
        portEXIT_CRITICAL_ISR(&pulseMutex);

        wheel_rotate_speed_mps = WHEEL_CIRCUMFERENCE / (psuedo_rotate_micros / 10000.0);
        if(xSemaphoreTake(accessSemaphore, (TickType_t)10 ) == pdTRUE) {
            wheel_rotate_speed_kmph = wheel_rotate_speed_mps * 3.6;

            xSemaphoreGive(accessSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void TaskPrint(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(accessSemaphore2, (TickType_t)10 ) == pdTRUE) {
            Serial.println(wheel_rotate_speed_kmph, 4);

            xSemaphoreGive(accessSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup(){
    Serial.begin(115200);

    /* Pin setting */
    pinMode(PULSE_INT_PIN, INPUT);

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskPulseWatchdog, "TaskPulseWatchdog", 4096, NULL, PRIORITY_2, NULL, CORE_0);
    xTaskCreateUniversal(TaskSpeedCalc, "TaskSpeedCalc", 4096, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskPrint, "TaskPrint", 4096, NULL, PRIORITY_0, NULL, CORE_0);
#endif

    /* Interrupt setting */
    attachInterrupt(digitalPinToInterrupt(PULSE_INT_PIN), PULSE_SIGNAL_ISR, RISING);

}

void loop(){}
