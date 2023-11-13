#define ESP32_DEVKIT
// #define UNO_ATMEGA328P

/* Main library */
#include "Arduino.h"

/* Communication library */
#include <SoftwareSerial.h>

/* Device library */
#define TINY_GSM_MODEM_BG96
#include <TinyGsmClient.h>

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
SoftwareSerial SerialForModem;
#elif defined(UNO_ATMEGA328P)
SoftwareSerial SerialForModem(BG96_RX, BG96_TX);
#endif
TinyGsm modem(SerialForModem);


void resetBG96() {
    digitalWrite(BG96_RST, LOW);
    delay(300);
    digitalWrite(BG96_RST, HIGH);
    delay(300);
    digitalWrite(BG96_RST, LOW);
}

String formatResponse(String str) {
    str.replace("\r\nOK\r\n", "[OK]");
    str.replace("\rOK\r", "[OK]");
    str.replace("\r\n", "\t");
    str.replace("\r", "\t");
    str.trim();
    return str;
}

String executeAT(String command, long timeout) {
    String buf;
    
    modem.sendAT(command);
    if (modem.waitResponse(timeout, buf) != 1) {
        return "ERROR";
    }

    return formatResponse(buf);
}

bool isOk(String str) {
    return str.indexOf("[OK]") >= 0;
}

void showModemInformation() {
    String res;

    Serial.println(F("> AT+GSN"));
    res = executeAT(F("+GSN"), 300);
    Serial.println(res);
    
    Serial.println(F("> AT+CIMI"));
    res = executeAT(F("+CIMI"), 300);
    Serial.println(res);
    
    Serial.println(F("> AT+QSIMSTAT?"));
    res = executeAT(F("+QSIMSTAT?"), 300);
    Serial.println(res);
    
}

void showNetworkInformation() {
    String res;
    
    Serial.println(F("> AT+QIACT?"));
    res = executeAT(F("+QIACT?"), 300);
    Serial.println(res);

    Serial.println(F("> AT+QCSQ"));
    res = executeAT(F("+QCSQ"), 300);
    Serial.println(res);
    
    Serial.println(F("> AT+COPS?"));
    res = executeAT(F("+COPS?"), 300);
    Serial.println(res);
    
    Serial.println(F("> AT+CGPADDR=1"));
    res = executeAT(F("+CGPADDR=1"), 300);
    Serial.println(res);
}

bool setupNetworkConfigurations() {
    String res;
    
    Serial.println(F("> AT+CGDCONT=1,\"IP\",\"soracom.io\",\"0.0.0.0\",0,0,0,0"));
    res = executeAT(F("+CGDCONT=1,\"IP\",\"soracom.io\",\"0.0.0.0\",0,0,0,0"), 300);
    Serial.println(res);
    bool setupPDP = isOk(res);

    Serial.println(F("> AT+QCFG=\"nwscanmode\",0,0"));
    res = executeAT(F("+QCFG=\"nwscanmode\",0,0"), 300);
    Serial.println(res);
    bool networkScanMode = isOk(res);
    
    Serial.println(F("> AT+QCFG=\"iotopmode\",0,0"));
    res = executeAT(F("+QCFG=\"iotopmode\",0,0"), 300);
    Serial.println(res);
    bool networkCategory = isOk(res);

    Serial.println(F("> AT+QCFG=\"nwscanseq\",00,1"));
    res = executeAT(F("+QCFG=\"nwscanseq\",00,1"), 300);
    Serial.println(res);
    bool scanSequence = isOk(res);

    return (setupPDP && networkScanMode && networkCategory && scanSequence);
}

int printPingResult(String input) {
    input.replace("+QPING: ", "");

    char buf[100] = { 0 };
    input.toCharArray(buf, 100);
    if (strlen(buf) <= 0) return 0;
    
    int result = atoi(strtok(buf, ","));
    if (result == 0) {
        Serial.print(F("Dest=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Bytes=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Time=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", TTL=")); Serial.print(strtok(NULL, ","));
        Serial.println();
    }
    else {
        Serial.print("(R)Error: "); Serial.println(result);
    }

    return result;
}

int printPingSummary(String input) {
    input.replace("+QPING: ", "");

    char buf[100] = { 0 };
    input.toCharArray(buf, 100);
    if (strlen(buf) <= 0) return 0;
    
    int result = atoi(strtok(buf, ","));
    if (result == 0) {
        Serial.print(F("Sent=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Received=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Lost=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Min=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Max=")); Serial.print(strtok(NULL, ","));
        Serial.print(F(", Avg=")); Serial.print(strtok(NULL, ","));
        Serial.println();
    }
    else {
        Serial.print("(S)Error: "); Serial.println(result);
    }

    return result;
}

void pingToSoracomNetwork() {
    String res;
    res = executeAT(F("+QPING=1,\"pong.soracom.io\",3,3"), 3000);

    if (isOk(res)) {
        modem.stream.readStringUntil('\n');
        String try1 = modem.stream.readStringUntil('\n');
        modem.stream.readStringUntil('\n');
        String try2 = modem.stream.readStringUntil('\n');
        modem.stream.readStringUntil('\n');
        String try3 = modem.stream.readStringUntil('\n');
        modem.stream.readStringUntil('\n');
        String summary = modem.stream.readStringUntil('\n');
        modem.stream.readStringUntil('\n');

        printPingResult(try1);
        printPingResult(try2);
        printPingResult(try3);
        printPingSummary(summary);
    }
}

void setup(){
    /* Serial setting */
    Serial.begin(115200);
#if defined(ESP32_DEVKIT)
    SerialForModem.begin(BG96_BAUDRATE, SWSERIAL_8N1, BG96_RX, BG96_TX, false, 256);
#elif defined(UNO_ATMEGA328P)
    SerialForModem.begin(BG96_BAUDRATE);
#endif

    /* LTE-M shield setting */
    Serial.println();
    Serial.println(F("****************************"));
    Serial.println(F("* Connectivity diagnostics *"));
    Serial.println(F("****************************"));

    Serial.println();
    Serial.print(F("--- Initializing modem, please wait for a while..."));
    pinMode(BG96_RST, OUTPUT);
    resetBG96();
    modem.restart();
    Serial.println(F("[OK]"));

    Serial.print(F("Target modem: "));
    String modemInfo = modem.getModemInfo();
    Serial.println(modemInfo);

    Serial.print(F("Testing AT Command: "));
    bool testATResult = modem.testAT();
    Serial.println((testATResult) ? "[OK]" : "[FAILED]");
    if (!testATResult) {
        Serial.println(F("Failed to execute test command, please RESET and retry later."));
        while(1);
    }

    Serial.println();
    Serial.println(F("--- Getting modem info..."));
    showModemInformation();

    Serial.println();
    Serial.println(F("--- Executing AT commands to connect SORACOM network..."));
    bool setupNetworkResult = setupNetworkConfigurations();
    if (!setupNetworkResult) {
        Serial.println(F("Failed to execute setup commands, please RESET and retry later."));
        while(1);
    }

    Serial.println();
    Serial.print(F("--- Connecting to cellular network, please wait for a while..."));
    bool networkConnect = modem.gprsConnect("soracom.io", "sora", "sora");
    bool networkConnected = modem.waitForNetwork();
    Serial.println((networkConnect && networkConnected) ? "[OK]" : "[FAILED]");
    if (!(networkConnect && networkConnected)) {
        Serial.println(F("Failed to connect cellular network."));
        Serial.println(F("Make sure active SIM has been inserted to the modem, and then check the antenna has been connected correctly."));
        Serial.println(F("Please RESET and retry later."));
        while(1);
    }

    Serial.println();
    Serial.println(F("--- Getting network info..."));
    showNetworkInformation();

    Serial.println();
    Serial.println(F("--- Conntectivity test: Ping to pong.soracom.io..."));
    pingToSoracomNetwork();

    Serial.println();
    Serial.println(F("--- Execution completed, please write your own sketch and enjoy it."));

}

void loop(){}


