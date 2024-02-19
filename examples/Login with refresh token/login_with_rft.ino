#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyESP32.h"
#include "config.h"
using namespace Spotify_types;

Spotify sp(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);
void setup() {
    Serial.begin(115200);
    connect_to_wifi();
}

void loop() {
    // put your main code here, to run repeatedly:
}
void connect_to_wifi(){
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected to WiFi\n");
}

