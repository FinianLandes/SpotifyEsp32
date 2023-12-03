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


response Spotify::RestApiGet(char rest_url[100]){
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
    Serial.print("GET currently_playing status: ");
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


response Spotify::RestApiPut(char rest_url[100]){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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


response Spotify::RestApiPost(char rest_url[100]){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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


response Spotify::currently_playing(){
  char link[] = "https://api.spotify.com/v1/me/player/currently-playing";
  return RestApiGet(link);
}


response Spotify::current_playback_state(){
  char link[] = "https://api.spotify.com/v1/me/player";
  return RestApiGet(link);
}


response Spotify::play_uri(String track_uri){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin("https://api.spotify.com/v1/me/player/play",_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));
  String payload;
  if(track_uri.startsWith("spotify:track:")){
    payload= "{\"uris\":[\"" + track_uri + "\"]}";
  }
  else{
    payload = "{\"context_uri\":\"" + track_uri + "\",\"offset\":{\"position\":0}}";
  }
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
    Serial.print("PUT play status: ");
    Serial.print(http_code);
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
        response_obj = play_uri(track_uri);
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


response Spotify::start_playback(){

  char link[] = "https://api.spotify.com/v1/me/player/play";

  response resp_obj = RestApiPut(link);
  return resp_obj;

}


response Spotify::pause_playback(){
  char link[] = "https://api.spotify.com/v1/me/player/pause";

  response resp_obj = RestApiPut(link);
  return resp_obj;

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

  char url[] = "https://api.spotify.com/v1/me/player/previous";
  return RestApiGet(url);

}


response Spotify::recently_played_tracks(int limit){
  response response_obj;
  init_response(&response_obj);
  HTTPClient http;
  String url = "https://api.spotify.com/v1/me/player/recently-played";
  url += "?limit="+String(limit);
  http.begin(url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));
 
  int http_code = http.GET();
  response_obj.status_code = http_code;
  String response = http.getString();
  if(_debug_on){
    Serial.print("GET recently_played_tracks status: ");
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(response);
  }
  if ((http_code >=200)&&(http_code<=299)){
    
    response_obj.reply = response;
  }
  else if(_retry<=3){
    _retry++;
    if(get_token()){
      response_obj = recently_played_tracks(limit);
    }
  }
  http.end();
  _retry = 0;
  return response_obj;
}


response Spotify::get_queue(){
  response response_obj;
  init_response(&response_obj);
  HTTPClient http;
  http.begin("https://api.spotify.com/v1/me/player/queue",_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));
 
  int http_code = http.GET();
  response_obj.status_code = http_code;
  if(_debug_on){
    Serial.print("GET get_queue status: ");
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
      response_obj = get_queue();
    }
  }
  http.end();
  _retry = 0;
  return response_obj;
}


response Spotify::transfer_playback(String device_id){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin("https://api.spotify.com/v1/me/player",_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "{\"device_ids\":[\"" + device_id + "\"]}";

  int http_code=http.PUT(payload);

  response_obj.status_code = http_code;
  String reply = "";
  DynamicJsonDocument doc(2000);

  if(http.getSize()>0){
    reply = http.getString();
    deserializeJson(doc, reply);
  }
  
  if(_debug_on){
    Serial.print("PUT play status: ");
    Serial.print(http_code);
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
        response_obj = transfer_playback(device_id);
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


response Spotify::seek_to_position(int time_ms){
  response response_obj;
  init_response(&response_obj);
  String url = "https://api.spotify.com/v1/me/player/seek";
  url += "?position_ms="+String(time_ms);

  HTTPClient http;
  http.begin(url, _spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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
    Serial.print("PUT seek_to_position status: ");
    Serial.print(http_code);
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
        response_obj = seek_to_position(time_ms);
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


response Spotify::repeat_mode(String mode){
  response response_obj;
  init_response(&response_obj);
  String url = "https://api.spotify.com/v1/me/player/repeat";
  url += "?state="+mode;

  HTTPClient http;
  http.begin(url, _spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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
    Serial.print("PUT repeat status: ");
    Serial.print(http_code);
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
        response_obj = repeat_mode(mode);
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


response Spotify::set_volume(int value){
  response response_obj;
  init_response(&response_obj);
  String url = "https://api.spotify.com/v1/me/player/volume";
  url += "?volume_percent="+String(value);

  HTTPClient http;
  http.begin(url, _spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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
    Serial.print("PUT set_volume status: ");
    Serial.print(http_code);
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
        response_obj = set_volume(value);
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


response Spotify::shuffle(bool mode){
  response response_obj;
  init_response(&response_obj);
  String state;
  if(mode){
    state = "true";
  }
  else{
    state = "false";
  }
  String url = "https://api.spotify.com/v1/me/player/shuffle";
  url += "?state=" + state;

  HTTPClient http;
  http.begin(url, _spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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
    Serial.print("PUT shuffle status: ");
    Serial.print(http_code);
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
        response_obj = shuffle(mode);
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


response Spotify::add_to_queue(String context_uri){
  response response_obj;
  init_response(&response_obj);
  String url = "https://api.spotify.com/v1/me/player/queue";
  url += "?uri="+urlEncode(context_uri);

  HTTPClient http;
  http.begin(url, _spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  String payload= "";
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
    Serial.print("PUT add_to_queue status: ");
    Serial.print(http_code);
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
        response_obj = add_to_queue(context_uri);
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