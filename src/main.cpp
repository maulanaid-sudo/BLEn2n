#include <Arduino.h>
#if defined ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#elif defined ESP8266
#include <ESP8266WiFi.h>
#define WIFI_MODE_STA WIFI_STA 
#else
#error "Unsupported platform"
#endif //ESP32
#include <QuickEspNow.h>

static const String msg = "Hello ESP-NOW!";

#define USE_BROADCAST 1 // Set this to 1 to use broadcast communication

#if USE_BROADCAST != 0
// set the MAC address of the receiver for unicast
static uint8_t receiver[] = { 0xB0, 0xB2, 0x1C, 0x51, 0xA1, 0xEC };
//static uint8_t receiver[] = { 0x78, 0x21, 0x84, 0xA6, 0xE1, 0xBC };
//static uint8_t receiver[] = { 0x08, 0xB6, 0x1F, 0x87, 0x30, 0x74 };

//08:b6:1f:87:30:74
#define DEST_ADDR receiver
#else //USE_BROADCAST != 1
#define DEST_ADDR ESPNOW_BROADCAST_ADDRESS 
#endif //USE_BROADCAST != 1

bool sent = true;

const unsigned int SEND_MSG_MSEC = 500;

void dataSent (uint8_t* address, uint8_t status) {
    sent = true;
    Serial.printf ("Message sent to " MACSTR ", status: %d\n", MAC2STR (address), status);
}

void setup () {
    Serial.begin (115200);
    WiFi.mode (WIFI_MODE_STA);
#if defined ESP32
    WiFi.disconnect (false, true);
#elif defined ESP8266
    WiFi.disconnect (false);
#endif //ESP32

    Serial.printf ("Connected to %s in channel %d\n", WiFi.SSID ().c_str (), WiFi.channel ());
    Serial.printf ("IP address: %s\n", WiFi.localIP ().toString ().c_str ());
    Serial.printf ("MAC address: %s\n", WiFi.macAddress ().c_str ());
    quickEspNow.begin (1, 0, false);
    quickEspNow.onDataSent (dataSent);
   // quickEspNow.onDataRcvd (dataReceived);
}

void loop () {
    static time_t lastSend = 0;
    static unsigned int counter = 0;

     // Sent flag is needed to wait for the message to be actually sent. Avoids messages dropping, maximizing throughput.
     // readyToSendData() is used to avoid sending messages too fast, which can lead to messages dropping.
    if (quickEspNow.readyToSendData() && sent && ((millis () - lastSend) > SEND_MSG_MSEC)) {
        lastSend = millis ();
        String message = String (msg) + " " + String (counter++);
        sent = false;
        if (!quickEspNow.send (DEST_ADDR, (uint8_t*)message.c_str (), message.length ())) {
            Serial.printf (">>>>>>>>>> Message sent\n");
        } else {
            Serial.printf (">>>>>>>>>> Message not sent\n");
            sent = true; // In case of error we need to set the flag to true to avoid blocking the loop
        }

    }
}
