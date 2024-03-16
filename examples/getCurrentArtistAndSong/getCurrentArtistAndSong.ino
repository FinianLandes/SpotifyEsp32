/*
    An example of how to authenticate with Spotify without using a refresh token and print Artist and Track via Serial

    This example is useful to get the refresh token for the first time. It can also be used to authenticate every time without using the refresh token.

    15.03.2024
    Created by: Finian Landes

    16.03.2024
    edited by: Sascha Seidel
        * added getting artist and trackname to print it Serial

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

Spotify sp(CLIENT_ID, CLIENT_SECRET);

void setup() {
    Serial.begin(115200);
    connect_to_wifi();
    
    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.println("Authenticated");
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
