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
#define SORACOM_APN "soracom.io"
#define SORACOM_USER "sora"
#define SORACOM_PASS "sora"

/* Json setting */
#include <ArduinoJson.h>
#define JSON_BUFF_SIZE 200
StaticJsonDocument<JSON_BUFF_SIZE> content;
unsigned long uptime_sec = 0;

struct EntityHeader {
    char start_line[40] = "POST / HTTP/1.1";
    char host[40] = "Host: uni.soracom.io";
    char content_type[40] = "Content-Type: application/json";
    char content_length[40] = "Content-Length: 0";
};

struct Request {
    char header[300];
    char body[JSON_BUFF_SIZE] = "";
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
    modem.gprsConnect(SORACOM_APN, SORACOM_USER, SORACOM_PASS);
}

void connect_endpoint(){
    if(!ctx.connect(END_POINT, 80)) {
        return;
    }
}

void create_request_header(){
    sprintf_P(header.host, PSTR("Host: %s"), END_POINT);
    sprintf_P(header.content_length, PSTR("Content-Length: %d"), strlen(request.body));
}

void send_request(unsigned long *id){
    if(xSemaphoreTake(xSemaphore1, (TickType_t)10) == pdTRUE) {
        content["id"] = *id;
        serializeJson(content, request.body);

        xSemaphoreGive(xSemaphore1);
    }

    create_request_header();
    sprintf(request.header, "%s\r\n%s\r\n%s\r\n%s\r\n", header.start_line, header.host, header.content_type, header.content_length);

    ctx.println(request.header);
    ctx.println(request.body);

    *id += 1;
}


void TaskSample1(void *pvParameters){
    unsigned long id = 0;

    for(;;) {
        connect_endpoint();
        send_request(&id);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskSample2(void *pvParameters){

    for(;;) {
        uptime_sec = millis() / 1000;
        if(xSemaphoreTake(xSemaphore2, (TickType_t)10) == pdTRUE) {
            content["uptime"] = uptime_sec;

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
    xTaskCreateUniversal(TaskSample1, "TaskSample1", 10240, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreateUniversal(TaskSample2, "TaskSample2", 4096, NULL, PRIORITY_0, NULL, CORE_0);
#endif

}

void loop(){}


