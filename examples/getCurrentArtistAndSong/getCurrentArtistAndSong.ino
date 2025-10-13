/*
    An example of how to authenticate with Spotify without using a refresh token and print Artist and Track via Serial

In this example your current track will be printed to the serial and as soon as you listen to a new track that tracks information will be printed.

    15.03.2024
    Created by: Finian Landes

    16.03.2024
    edited by: Sascha Seidel
        * added getting artist and trackname to print it Serial

    11.10.2025
    edited by: Finian Landes
        * Updated login to work with version 3.x.x

    Documentation: https://github.com/FinianLandes/Spotify_Esp32
*/
// Include the required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>

char* SSID = "YOUR WIFI SSID";
const char* PASSWORD = "YOUR WIFI PASSWORD";
const char* CLIENT_ID = "YOUR CLIENT ID FROM THE SPOTIFY DASHBOARD";
const char* CLIENT_SECRET = "YOUR CLIENT SECRET FROM THE SPOTIFY DASHBOARD";
const char* REFRESH_TOKEN = "YOUR REFRESH TOKEN";

Spotify sp(CLIENT_ID, CLIENT_SECRET);

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
    static String lastArtist;
    static String lastTrackname;
    
    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();
    
    if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        lastArtist = currentArtist;
        Serial.println("Artist: " + lastArtist);
    }
    
    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        lastTrackname = currentTrackname;
        Serial.println("Track: " + lastTrackname);
    }
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
