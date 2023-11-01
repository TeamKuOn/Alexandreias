current_dir=$(pwd)

dir_name="$1"
mkdir $dir_name
cd $dir_name

## Inirialized project
board="$2"
baud=115200
frameworks="framework=arduino"

pio project init \
    --board $board \
    --project-option $frameworks  

## add ignore dir
echo '
# PlatformIO
.pio
.pioenvs
.piolibdeps

# VSCode
.vscode
' >> "./.gitignore"

## Create frame src file
main_file="./src/main.cpp"
touch $main_file
echo '/* Main library */
#include "Arduino.h"

/* Communication library */

/* Device library */

/* Function declaration for Task Management */
void TaskSample1(void *pvParameters);
void TaskSample2(void *pvParameters);

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITIY_1 1
#define PRIORITIY_0 0

/* Function declaration for Interrupt Management */
void Sample_ISR();

/* Semaphore declaration */
SemaphoreHandle_t xSemaphore;

/* Global variable */
#define INT_PIN 2


void setup(){
    /* Serial setting */
    Serial.begin(115200);

    /* Semaphore setting */
    if((xSemaphore = xSemaphoreCreateMutex()) != NULL) {
        xSemaphoreGive((xSemaphore));
    }

    /* Pin setting */
    pinMode(INT_PIN, INPUT);

    /* Task setting */
    xTaskCreateUniversal(TaskSample1, "TaskSample1", 1024, NULL, PRIORITIY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskSample2, "TaskSample2", 1024, NULL, PRIORITIY_0, NULL, CORE_0);

    /* Interrupt setting */
    attachInterrupt(digitalPinToInterrupt(INT_PIN), Sample_ISR, FALLING);

}

void loop(){}

void Sample_ISR() {
    if(xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE) {
        // Write some code

        xSemaphoreGiveFromISR(xSemaphore, NULL);
    }
}

void TaskSample1(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore, (TickType_t)1) == pdTRUE) {
            Serial.println("1");

            xSemaphoreGive(xSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskSample2(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore, (TickType_t)1) == pdTRUE) {
            Serial.println("2");

            xSemaphoreGive(xSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

' >> $main_file

cd $current_dir