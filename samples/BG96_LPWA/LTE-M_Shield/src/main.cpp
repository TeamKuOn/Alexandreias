#define ESP32_DEVKIT
// #define UNO_ATMEGA328P

/* Main library */
#include "Arduino.h"

/* Communication library */
#include <SoftwareSerial.h>

/* Device library */
#define TINY_GSM_MODEM_BG96
#include <TinyGsmClient.h>

/* Task declaration */
#define CORE_0 0
#define CORE_1 1

#define PRIORITY_1 1
#define PRIORITY_0 0

/* Semaphore & Mutex declaration */
SemaphoreHandle_t xSemaphore1 = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphore2 = xSemaphoreCreateMutex();

/* LTE-M shield setting */
#if defined(ESP32_DEVKIT)
#define BG96_RX 14
#define BG96_TX 15
#define BG96_RST 33
#elif defined(UNO_ATMEGA328P)
#define BG96_RX 10
#define BG96_TX 11
#define BG96_RST 15
#endif

#define BG96_BAUDRATE 9600

#if defined(ESP32_DEVKIT)
SoftwareSerial LTE_M_shield;
#elif defined(UNO_ATMEGA328P)
SoftwareSerial LTE_M_shield(BG96_RX, BG96_TX);
#endif
TinyGsm modem(LTE_M_shield);
TinyGsmClient ctx(modem);

#define END_POINT "uni.soracom.io"

/* Json setting */
#include <ArduinoJson.h>
StaticJsonDocument<200> content;
unsigned long uptime_sec = 0;

struct EntityHeader {
    char start_line[40] = "POST / HTTP/1.1";
    char host[40] = "Host: uni.soracom.io";
    char content_type[40] = "Content-Type: application/json";
    char content_length[40] = "Content-Length: 0";
};

struct Request {
    char header[300];
    char body[120] = "";
};

struct EntityHeader header;
struct Request request;

void hardware_reset_BG96(){
    pinMode(BG96_RST, OUTPUT);
    digitalWrite(BG96_RST, LOW);
    delay(300);
    digitalWrite(BG96_RST, HIGH);
    delay(300);
    digitalWrite(BG96_RST, LOW);

    modem.restart();
    modem.gprsConnect("soracom.io", "sora", "sora");
}

void connect_endpoint(){
    unsigned int timeout = 240;
    unsigned long start = millis();
    while(!ctx.connect(END_POINT, 80)) {
        delay(10);
        if(millis() - start > timeout) {
            return;
        }
    }
}

void create_request_header(){
    sprintf_P(header.host, PSTR("Host: %s"), END_POINT);
    sprintf_P(header.content_length, PSTR("Content-Length: %d"), strlen(request.body));
}

void send_request(){
    create_request_header();
    if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
        sprintf(request.header, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", header.start_line, header.host, header.content_type, header.content_length);
        xSemaphoreGive(xSemaphore1);
    }

    ctx.println(request.header);
    ctx.println(request.body);
}

void recieve_response(){
    String line = ctx.readStringUntil('\n');
    while(ctx.connected()) {
        if(line == "\r") {
            Serial.println("headers received");
            break;
        }
    }

    ctx.stop();
}

void TaskSample1(void *pvParameters){

    for(;;) {
        connect_endpoint();
        send_request();
        recieve_response();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskSample2(void *pvParameters){

    for(;;) {
        uptime_sec = millis() / 1000;
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            content["uptime"] = uptime_sec;
            serializeJson(content, request.body);

            xSemaphoreGive(xSemaphore2);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup(){
    /* Serial setting */
    Serial.begin(115200);
#if defined(ESP32_DEVKIT)
    LTE_M_shield.begin(BG96_BAUDRATE, SWSERIAL_8N1, BG96_RX, BG96_TX, false, 256);
#elif defined(UNO_ATMEGA328P)
    LTE_M_shield.begin(BG96_BAUDRATE);
#endif

    /* LTE-M shield setting */
    hardware_reset_BG96();

    /* Task setting */
#if defined(ESP32_DEVKIT)
    xTaskCreateUniversal(TaskSample1, "TaskSample1", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskSample2, "TaskSample2", 1024, NULL, PRIORITY_0, NULL, CORE_0);
#endif

}

void loop(){}


