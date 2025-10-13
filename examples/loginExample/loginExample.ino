/*
    An example of how to authenticate with Spotify.

    This example is useful authenticate in different ways.

    24.03.2024
    Created by: Finian Landes

    11.10.2025
    edited by: Finian Landes
        * Updated login to work with version 2.x.x

    Documentation: https://github.com/FinianLandes/Spotify_Esp32
*/
// Include the required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>

char* SSID = "YOUR WIFI SSID";
const char* PASSWORD = "YOUR WIFI PASSWORD";
const char* CLIENT_ID = "YOUR CLIENT ID FROM THE SPOTIFY DASHBOARD";//leave these empty if you want to set the credentials during runtime
const char* CLIENT_SECRET = "YOUR CLIENT SECRET FROM THE SPOTIFY DASHBOARD";//leave these empty if you want to set the credentials during runtime

Spotify sp(CLIENT_ID, CLIENT_SECRET);
//If you already have a refresh token uncomment following line
//Spotify sp(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);

void setup() {
    Serial.begin(115200);
    connect_to_wifi();
    // Uncomment following line if you want to enable debugging:
    // sp.set_log_level(SPOTIFY_LOG_DEBUG);
    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.printf("Authenticated! Refresh token: %s\n", sp.get_user_tokens().refresh_token);
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
