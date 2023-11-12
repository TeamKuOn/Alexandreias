current_dir=$(pwd)

dir_name="$1"
mkdir $dir_name
cd $dir_name

prj_name="$2"
mkdir $prj_name
cd $prj_name

## Inirialized project
board="$3"
baud=115200
frameworks="framework=arduino"

pio project init \
    --board $board \
    --project-option $frameworks  

echo 'upload_speed = 115200
monitor_speed = 115200
' >> "./platformio.ini"

## Add ignoring dir
IGNORE_FILE="./.gitignore"
if [ ! -s $IGNORE_FILE ]; then
    echo '
# PlatformIO
.pio
.pioenvs
.piolibdeps

# VSCode
.vscode
' >> $IGNORE_FILE
else
    echo "File $IGNORE_FILE already exists."
fi

## Create frame src file
MAIN_FILE="./src/main.cpp"
touch $MAIN_FILE
if [ ! -s $MAIN_FILE ]; then
    echo '#define ESP32_DEVKIT

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
portMUX_TYPE Mutex = portMUX_INITIALIZER_UNLOCKED;

/* Global variable */
#define INT_PIN 2



void IRAM_ATTR Sample_ISR() {
    portENTER_CRITICAL_ISR(&Mutex);

    // Write some code

    portEXIT_CRITICAL_ISR(&Mutex);
}

void TaskSample1(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
            Serial.println("1");

            xSemaphoreGive(xSemaphore1);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskSample2(void *pvParameters){

    for(;;) {
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            Serial.println("2");

            xSemaphoreGive(xSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void setup(){
    /* Serial setting */
    Serial.begin(115200);

    /* Pin setting */
    pinMode(INT_PIN, INPUT);

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskSample1, "TaskSample1", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskSample2, "TaskSample2", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif

    /* Interrupt setting */
    attachInterrupt(digitalPinToInterrupt(INT_PIN), Sample_ISR, FALLING);

}

void loop(){}

' >> $MAIN_FILE
    echo "File $MAIN_FILE created."
else
    echo "File $MAIN_FILE already exists."
fi

cd $current_dir