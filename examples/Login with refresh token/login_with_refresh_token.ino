/*
    An example of how to use the Spotify library using a refresh token.

    This example is useful as it does not require the user to authenticate in the browser every time they want to use the library.

    20.02.2024
    Created by: Finian Landes

    Documentation: https://github.com/FinianLandes/Spotify_Esp32
*/
// Include the required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyESP32.h>
const char* SSID = "YOUR WIFI SSID";
const char* PASSWORD = "YOUR WIFI PASSWORD";
const char* CLIENT_ID = "YOUR CLIENT ID FROM THE SPOTIFY DASHBOARD";
const char* CLIENT_SECRET = "YOUR CLIENT SECRET FROM THE SPOTIFY DASHBOARD";
const char* REFRESH_TOKEN = "YOUR REFRESH TOKEN";

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

