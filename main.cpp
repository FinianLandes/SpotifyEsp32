#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyESP32.h"
const char* ssid = "Rebweg10D";
const char* password = "Bitte_eintreten";
const char* client_id = "fc0799459cf04f1aab6bb3f861964e77";
const char* client_secret = "9ca64f76f7b54ecdaf3ae78585ff8b0f";
int port = 8000;
const char* refresh_token = "AQAlYkKcfHdrul3u451q32Wl0Fy4HWcmh0Mbyerl1-8V3eKmDADpYhCYBXAeAHbPjadhNqULx8cNyfhUkwSh-dmIo27IY1kzSSee4xxtptOvuueBRUdiiCDL-nghpkB0il1qbYIhcBSGfUKU_bjL3588piUWrTKZxPKeRnRirXftxJ5jB_vg5Vrb1tyzLEriAHlH2tCMos4Yh1v7bKilERVijEIctixNnNlLUvBiDOVsX-BfdyRO158SWrQ0P6yrPOhNbLxvMhY-Dp7L8gYsZBoFi1BnDSxJbEPvmuDMh28a";

using namespace Spotify_types;
void connect_to_wifi();
//Client without Auth token
//Spotify sp(client_id, client_secret, port, true);
//Client with Auth token
Spotify sp(client_id, client_secret, refresh_token, true);
void setup() {
    Serial.begin(115200);
    connect_to_wifi();
    // Uncomment the following few lines to get the auth token
    /*sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }*/
    print_response(sp.pause_playback());
}

void loop() {

}
void connect_to_wifi(){
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to WiFi");
}

