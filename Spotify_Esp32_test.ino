#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "SpotifyESP32.h"

const char* ssid = "YOUR SSID";
const char* password = "YOUR WIFI PASSWORD";
char* client_id = "YOUR CLIENT ID";
char* client_secret = "YOUR CLIENT SECRET";
char* redirect_uri = "http://localhost:8080";//Set redirect uri on dev dashboard
char* refresh_token="YOUR CLIENT SECRET";

using namespace Spotify_types;

Spotify sp(refresh_token, redirect_uri, client_id, client_secret, false);//Set last Parameter to true to get more debug information
void setup() {
  Serial.begin(9600);
  connectToWifi();
  Serial.print("Current Track: ");
  Serial.print(sp.current_track_name());
  Serial.print(" by ");
  Serial.println(sp.current_artist_names());

  Serial.println(sp.convert_id_to_uri(sp.current_track_id(),TYPE_TRACK));
  print_response(sp.shuffle(SHUFFLE_ON));

}

void loop() {
  // put your main code here, to run repeatedly:

}
void connectToWifi(){
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}