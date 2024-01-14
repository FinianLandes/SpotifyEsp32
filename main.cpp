
#include <WiFi.h>
#include <Arduino.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* client_id = "YOUR_CLIENT_ID";
const char* client_secret = "YOUR_CLIENT_SECRET";
const char* redirect_uri = "YOUR_REDIRECT_URI";
const char* refresh_token="YOUR_REFSRESH_TOKEN";

using namespace Spotify_types;

Spotify sp(refresh_token, redirect_uri, client_id, client_secret, false);

char song_id[] = "2NTCi4wGypj56t843jb3Mt";

void connect_to_wifi();

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

void setup() {
  Serial.begin(115200);
  connect_to_wifi();
  print_result(sp.currently_playing());
  char context_uri[SIZE_OF_URI];
  sp.convert_id_to_uri(id,TYPE_TRACK,context_uri);
  sp.start_resume_playback(context_uri);

}
void loop() {
}

