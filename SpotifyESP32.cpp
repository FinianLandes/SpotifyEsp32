#include "SpotifyESP32.h"

namespace Spotify_types{
  bool SHUFFLE_ON = true;
  bool SHUFFLE_OFF = false;
  String REPEAT_OFF = "off";
  String REPEAT_TRACK = "track";
  String REPEAT_CONTEXT = "context";
  String TYPE_ALBUM = "album";
  String TYPE_ARTIST = "artist";
  String TYPE_TRACK = "track";
  String TYPE_PLAYLIST = "playlist";
}


Spotify::Spotify(char* refresh_token, char* redirect_uri, char* client_id, char* client_secret,bool debug_on){
  _retry = 0;
  _refresh_token = refresh_token;
  _redirect_uri = redirect_uri;
  _client_id = client_id;
  _client_secret = client_secret;
  _debug_on = debug_on;
}


response Spotify::RestApiGet(char rest_url[_size_of_possibly_large_char]){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));
  int http_code = http.GET();

  response_obj.status_code = http_code;

  if(_debug_on){
    Serial.print("GET status: ");
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(http.getString());
  }
  if ((http_code >=200)&&(http_code<=299)){
    String response = http.getString();
    response_obj.reply = response;
  }
  else if(_retry<=3){
    _retry++;
    if(get_token()){
      response_obj = currently_playing();
    }
  }
  http.end();
  _retry = 0;

  return response_obj;
}
response Spotify::RestApiPut(char rest_url[100], String payload){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  http.addHeader("content-Length", String(payload.length()));
  int http_code=http.PUT(payload);

  response_obj.status_code = http_code;
  String reply = "";
  DynamicJsonDocument doc(2000);

  if(http.getSize()>0){
    reply = http.getString();
    deserializeJson(doc, reply);
  }

  if(_debug_on){
    //TODO: Add rexex ([^\/]+$) to extract last text (pause/play etc.) from uri and put into debug info
    Serial.print("PUT status: ");
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(reply);
  }
  if ((http_code >= 200)&&(http_code <= 299)){
    response_obj.reply = "Success";    
  }
  else if(_retry<=3){
    String message = doc["error"]["message"].as<String>();
    if(message == "Only valid bearer authentication supported"){
      _retry++;
      if(get_token()){
        response_obj = RestApiPut(rest_url);
      }
    }
    else{
      response_obj.reply = message;
    }
  }
  http.end();
  _retry = 0;

  return response_obj;
}
response Spotify::RestApiPost(char rest_url[100], String payload){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  
  http.addHeader("content-Length", String(payload.length()));
  int http_code=http.POST(payload);

  response_obj.status_code = http_code;
  String reply = "";
  DynamicJsonDocument doc(2000);

  if(http.getSize()>0){
    reply = http.getString();
    deserializeJson(doc, reply);
  }

  if(_debug_on){
    //TODO: Add rexex ([^\/]+$) to extract last text (pause/play etc.) from uri and put into debug info
    Serial.print("PUT start_playback status: ");
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(reply);
  }
  if ((http_code >= 200)&&(http_code <= 299)){
    response_obj.reply = "Success";    
  }
  else if(_retry<=3){
    String message = doc["error"]["message"].as<String>();
    if(message == "Only valid bearer authentication supported"){
      _retry++;
      if(get_token()){
        response_obj = RestApiPost(rest_url);
      }
    }
    else{
      response_obj.reply = message;
    }
  }
  http.end();
  _retry = 0;

  return response_obj;
}
//Untested
response Spotify::RestApiDelete(char rest_url[100], String payload){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url, _spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(_access_token));

  int http_code = http.sendRequest("DELETE", payload); 

  response_obj.status_code = http_code;
  String reply = "";
  DynamicJsonDocument doc(2000);

  if (http.getSize() > 0) {
    reply = http.getString();
    deserializeJson(doc, reply);
  }

  if (_debug_on) {
    // TODO: Add regex ([^\/]+$) to extract last text (pause/play etc.) from uri and put into debug info
    Serial.print("DELETE status: ");
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(reply);
  }

  if ((http_code >= 200) && (http_code <= 299)) {
    response_obj.reply = "Success";
  } else if (_retry <= 3) {
    String message = doc["error"]["message"].as<String>();
    if (message == "Only valid bearer authentication supported") {
      _retry++;
      if (get_token()) {
        response_obj = RestApiDelete(rest_url); 
      }
    } else {
      response_obj.reply = message;
    }
  }

  http.end();
  _retry = 0;

  return response_obj;

}
//Player
response Spotify::currently_playing(){
  char url[] = "https://api.spotify.com/v1/me/player/currently-playing";
  return RestApiGet(url);
}
response Spotify::current_playback_state(){
  char url[] = "https://api.spotify.com/v1/me/player";
  return RestApiGet(url);
}
response Spotify::play_uri(String track_uri){
  char url[] = "https://api.spotify.com/v1/me/player/play";
  
  String payload;
  if(track_uri.startsWith("spotify:track:")){
    payload = "{\"uris\":[\"" + track_uri + "\"]}";
  }
  else{
    payload = "{\"context_uri\":\"" + track_uri + "\",\"offset\":{\"position\":0}}";
  }
  return RestApiPut(url, payload);
}
response Spotify::start_playback(){
  char url[] = "https://api.spotify.com/v1/me/player/play";

  return RestApiPut(url);
}
response Spotify::pause_playback(){
  char url[] = "https://api.spotify.com/v1/me/player/pause";

  return RestApiPut(url);
}
response Spotify::skip(){
  char url[] = "https://api.spotify.com/v1/me/player/next";

  return RestApiPost(url);
}
response Spotify::previous(){
  char url[] = "https://api.spotify.com/v1/me/player/previous";
  
  return RestApiPost(url);

}
response Spotify::available_devices(){
  char url[] = "https://api.spotify.com/v1/me/player/devices";

  return RestApiGet(url);
}
response Spotify::recently_played_tracks(int limit){
  String url = "https://api.spotify.com/v1/me/player/recently-played";
  url += "?limit="+String(limit);

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);
}
response Spotify::get_queue(){
  char url[] = "https://api.spotify.com/v1/me/player/queue";
 
  return RestApiGet(url);
}
response Spotify::transfer_playback(String device_id){
  char url[] = "https://api.spotify.com/v1/me/player";
  String payload= "{\"device_ids\":[\"" + device_id + "\"]}";
  
  return RestApiPut(url, payload);
}
response Spotify::seek_to_position(int time_ms){
  String url = "https://api.spotify.com/v1/me/player/seek";
  url += "?position_ms="+String(time_ms);

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiPut(url_char_array);
}
response Spotify::repeat_mode(String mode){
  String url = "https://api.spotify.com/v1/me/player/repeat";
  url += "?state="+mode;

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiPut(url_char_array);
}
response Spotify::set_volume(int value){
  String url = "https://api.spotify.com/v1/me/player/volume";
  url += "?volume_percent="+String(value);

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiPut(url_char_array);
}
response Spotify::shuffle(bool mode){
  String state;
  if(mode){
    state = "true";
  }
  else{
    state = "false";
  }
  String url = "https://api.spotify.com/v1/me/player/shuffle";
  url += "?state=" + state;

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiPut(url_char_array);
}
response Spotify::add_to_queue(String context_uri){
  response response_obj;
  init_response(&response_obj);
  String url = "https://api.spotify.com/v1/me/player/queue";
  url += "?uri="+context_uri;

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiPost(url_char_array);
}
//Albums
response Spotify::get_album(String id){
  String url = "https://api.spotify.com/v1/albums";
  url += "/" + id;

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);
}
response Spotify::get_albums(String ids){
  String url = "https://api.spotify.com/v1/albums";
  url  += "?ids=" + ids;

  char url_char_array[_size_of_possibly_large_char]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);

}
response Spotify::get_album_tracks(String id, int limit, int offset){
  String url = "https://api.spotify.com/v1/albums/";
  url += id + "/tracks?limit=" + String(limit) + "&offset=" + String(offset);

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);

}
response Spotify::get_users_saved_albums(int limit, int offset){
  String url = "https://api.spotify.com/v1/me/albums";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);

}
response Spotify::save_albums_for_current_user(String ids){
  char url[] = "https://api.spotify.com/v1/me/albums";

  return RestApiPut(url, ids);
}
response Spotify::remove_users_saved_albums(String ids){
  char url[] = "https://api.spotify.com/v1/me/albums";

  return RestApiDelete(url, ids);
}
response Spotify::check_useres_saved_albums(String ids){
  String url = "https://api.spotify.com/v1/me/albums/contains";
  url  += "?ids=" + ids;

  char url_char_array[_size_of_possibly_large_char]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);
}
response Spotify::get_new_releases(String country, int limit, int offset){
  String url = "https://api.spotify.com/v1/browse/new-releases";
  url  += "?country=" + country + "&limit=" + String(limit) + "&offset=" + String(offset);

  char url_char_array[100]; 
  url.toCharArray(url_char_array, sizeof(url_char_array));

  return RestApiGet(url_char_array);
}
void Spotify::init_response(response* response_obj){
  response_obj -> status_code = -1;
  response_obj -> reply ="If you see this something went wrong";
}
void print_response(response response_obj){
  Serial.print("Status: ");
  Serial.println(response_obj.status_code);
  Serial.print("Reply: ");
  Serial.println(response_obj.reply);
}
bool Spotify::get_token(){
  bool reply = false;
  HTTPClient http;
  String url = "https://accounts.spotify.com/api/token";

  String authorization = String(_client_id) + ":" + String(_client_secret);
  authorization.trim();
  authorization = "Basic " + base64::encode(authorization);
  http.begin(url,_spotify_root_ca);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", authorization);

  String payload = "grant_type=refresh_token&refresh_token="+String(_refresh_token);
  int http_code = http.POST(payload);
  if(_debug_on){
    Serial.print("POST token status: ");
    Serial.println(http_code);
  }
  if ((http_code >=200)&&(http_code<=299)) {
    String response = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    _access_token = doc["access_token"].as<String>();
    reply = true;
  }
  else{ 
    reply = false;
  }
  http.end();
  return reply;
}
String Spotify::current_track_name(){
  String track_name = "Something went wrong";
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    track_name = doc["item"]["name"].as<String>();
  }
  return track_name;

}
String Spotify::current_track_id(){
  String track_id = "Something went wrong";
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    track_id = doc["item"]["id"].as<String>();
  }
  return track_id;
}
String Spotify::current_device_id(){
  String device_id = "Something went wrong";
  response data = available_devices();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(2000);
    deserializeJson(doc,data.reply);
    JsonArray devices = doc["devices"].as<JsonArray>();
    for (JsonVariant device : devices) {
      JsonObject deviceObj = device.as<JsonObject>();

      if (deviceObj["is_active"].as<bool>()) {
        device_id = deviceObj["id"].as<String>();
        break;
      }
    }  
  }
  return device_id;
}
String Spotify::current_artist_names(){
  String artist_names = "Something went wrong";
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    JsonArray array = doc["item"]["artists"];
    int len = array.size();
    artist_names = "";
    for (int i = 0; i<len; i++){
      artist_names += array[i]["name"].as<String>();
      if (i<len-1){
        artist_names += ", ";
      }
    }
  }
  return artist_names;
}
bool Spotify::is_playing(){
  bool is_playing = true;
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    is_playing = doc["is_playing"].as<bool>();
  }
  return is_playing;
}
String Spotify::convert_id_to_uri(String id, String type){
  String uri = "spotify:"+type+":"+id;
  return uri;
}

String Spotify::comma_separated_string_to_json(String list) {
  DynamicJsonDocument doc(512);
  String token;
  int pos = 0;

  while ((pos = list.indexOf(',')) != -1) {
    token = list.substring(0, pos);
    doc.add(token);
    list.remove(0, pos + 1);
  }

  doc.add(list);

  String list_doc;
  serializeJson(doc, list_doc);
  return list_doc;
}
