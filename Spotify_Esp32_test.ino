#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "SpotifyESP32.h"

const char* ssid = "Rebweg10D";
const char* password = "Bitte_eintreten";
char* client_id = "fc0799459cf04f1aab6bb3f861964e77";
char* client_secret = "9ca64f76f7b54ecdaf3ae78585ff8b0f";
char* redirect_uri = "http://localhost:8080";
char* refresh_token="AQAichrGMfjjDHYWIAENJYyWoi_KpzLZ93_HSS30J8zULeuHRPmF9-Wh3aSEd5ju8tloWGDZ9GJodjRH51PRUPvTqPg855b3fdmQ8JifmdNAEfSapfhkihTKKXveJxie00o";

using namespace Spotify_types;

Spotify sp(refresh_token, redirect_uri, client_id, client_secret, true);
String song_id = "2NTCi4wGypj56t843jb3Mt";
String album_id = "7iLuHJkrb9KHPkMgddYigh";
String album_ids = "7iLuHJkrb9KHPkMgddYigh,4KAtLRVIfB0bKnRY01dveY,2SxoeF005n621Jca66RRdu";
void setup() {
  Serial.begin(9600);
  connectToWifi();

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
void test_player(){
  Serial.print("Currently Playing: ");
  print_response(sp.currently_playing());

  Serial.print("Play track: ");
  print_response(sp.play_uri(sp.convert_id_to_uri(song_id,TYPE_TRACK)));

  Serial.print("Pause: ");
  print_response(sp.pause_playback());

  Serial.print("Play: ");
  print_response(sp.start_playback());

  Serial.print("Skip: ");
  print_response(sp.skip());

  Serial.print("Previous: ");
  print_response(sp.previous());

  Serial.print("Available Devices: ");
  print_response(sp.available_devices());

  Serial.print("Playback state: ");
  print_response(sp.current_playback_state());

  Serial.print("Recently played: ");
  print_response(sp.recently_played_tracks(1));

  Serial.print("Seek to pos: ");
  print_response(sp.seek_to_position(500));

  Serial.print("Add to queue: ");
  print_response(sp.add_to_queue(sp.convert_id_to_uri(song_id,TYPE_TRACK)));
  

  Serial.print("Get Queue: ");
  print_response(sp.get_queue());

  Serial.print("Repeat: ");
  print_response(sp.repeat_mode(REPEAT_CONTEXT));

  Serial.print("Shuffle off: ");
  print_response(sp.shuffle(SHUFFLE_OFF));

  Serial.print("Transfer Playback: ");
  print_response(sp.transfer_playback(sp.current_device_id()));

  Serial.print("Set Volume: ");
  print_response(sp.set_volume(50));
}
void test_albums(){
  Serial.print("Get Album: ");
  print_response(sp.get_album(album_id));

  Serial.print("Get Albums: ");
  print_response(sp.get_albums(album_ids));

  Serial.print("Get Album Tracks: ");
  print_response(sp.get_album_tracks(album_id, 1, 0));

  Serial.print("Get Saved Albums: ");
  print_response(sp.get_users_saved_albums(1, 0));

  Serial.print("Remove Saved Albums: ");
  print_response(sp.remove_users_saved_albums(sp.comma_separated_string_to_json(album_ids)));

  Serial.print("Save Albums: ");
  print_response(sp.save_albums_for_current_user(sp.comma_separated_string_to_json(album_ids))); 
 
  Serial.print("Check if Album Saved: ");
  print_response(sp.check_useres_saved_albums(album_ids));

  Serial.print("Get Releases: ");
  print_response(sp.get_new_releases("CH", 1, 0));
}