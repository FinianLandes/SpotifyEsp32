#include "SpotifyEsp32.h"
namespace Spotify_types {
  bool SHUFFLE_ON = true;
  bool SHUFFLE_OFF = false;
  const char* REPEAT_OFF = "off";
  const char* REPEAT_TRACK = "track";
  const char* REPEAT_CONTEXT = "context";
  const char* TYPE_ALBUM = "album";
  const char* TYPE_ARTIST = "artist";
  const char* TYPE_TRACK = "track";
  const char* TYPE_PLAYLIST = "playlist";
  const char* TYPE_SHOW = "show";
  const char* TYPE_EPISODE = "episode";
  const char* TYPE_AUDIOBOOK = "audiobook";
  const char* TOP_TYPE_ARTIST = "artists";
  const char* TOP_TYPE_TRACKS = "tracks";
  const char* GROUP_ALBUM = "album";
  const char* GROUP_SINGLE = "single";
  const char* GROUP_APPEARS_ON = "appears_on";
  const char* GROUP_COMPILATION = "compilation";
  const char* TIME_RANGE_SHORT = "short_term";
  const char* TIME_RANGE_MEDIUM = "medium_term";
  const char* TIME_RANGE_LONG = "long_term";
  const char* FOLLOW_TYPE_ARTIST = "artist";
  const char* FOLLOW_TYPE_USER = "user";
}

Spotify::Spotify(const char* client_id, const char* client_secret, int server_port, bool debug_on, int max_num_retry){
  _retry = 0;
  if(!(client_id && client_secret)){
    _no_credentials = true;
  }else{
    strncpy(_client_id, client_id,sizeof(_client_id));
    strncpy(_client_secret, client_secret,sizeof(_client_secret));
  }
  _port = server_port;
  _debug_on = debug_on;
  if(max_num_retry>0){
    _max_num_retry = max_num_retry;
  }
  else{
    _max_num_retry = 1;
  }
  #ifndef DISABLE_WEB_SERVER
  _server = new WebServer(server_port);
  #else
  _server = nullptr;
  #endif
}

Spotify::Spotify(const char* client_id, const char* client_secret, const char* refresh_token, int server_port,bool debug_on, int max_num_retry){
  _retry = 0;
  if(!(client_id && client_secret && refresh_token)){
    _no_credentials = true;
  }else{
    strncpy(_client_id, client_id,sizeof(_client_id));
    strncpy(_client_secret, client_secret,sizeof(_client_secret));
    strncpy(_refresh_token, refresh_token,sizeof(_refresh_token));
  }
  _debug_on = debug_on;
  _port = server_port;
  if(max_num_retry > 0){
    _max_num_retry = max_num_retry;
  }
  else{
    _max_num_retry = 1;
  }
  #ifndef DISABLE_WEB_SERVER
  _server = new WebServer(server_port);
  #else
  _server = nullptr;
  #endif
}
Spotify::~Spotify(){
  #ifndef DISABLE_WEB_SERVER
    delete _server;
  #endif
  if(_debug_on){
    Serial.println("Object Destructed");
  }
}
//Login Webserver Functions
#ifndef DISABLE_WEB_SERVER
std::function<void()> callback_fn_refresh(Spotify *spotify) {
  return [spotify]() {
        if(spotify->_server != nullptr)
        return spotify->server_on_refresh();
    };
};
std::function<void()> callback_fn_root(Spotify *spotify) {
  return [spotify]() {
        if(spotify->_server != nullptr)
        return spotify->server_on_root();
    };
};
std::function<void()> callback_fn_response(Spotify *spotify) {
  return [spotify]() {
        if(spotify->_server != nullptr)
        return spotify->server_on_response();
    };
};
void Spotify::server_on_root() {
  if((strcmp(_client_id, "") == 0 || !_client_id || strcmp(_client_secret, "") == 0 || !_client_secret)){
    _server->send(200,"text/html", String(_credentials_input));
  }else{
    _server->sendHeader("Location", String(_redirect_uri)+"callback");
    _server->send(302, "text/plain", "");
  }
}
void Spotify::server_on_response(){
  if(_server->args() != 0){
    if(_server->hasArg("id")){
      strncpy(_client_id, _server->arg("id").c_str(), sizeof(_client_id));
    }
    if(_server->hasArg("secret")){
      strncpy(_client_secret, _server->arg("secret").c_str(), sizeof(_client_secret));
    }
    if(_server->hasArg("token")){
      strncpy(_refresh_token, _server->arg("token").c_str(), sizeof(_refresh_token));
    }
    if (strcmp(_refresh_token, "") != 0) {
      _server->send(200, "text/html", "Credentials successfully set!");
      _server->stop();
    } else if (strcmp(_refresh_token, "") == 0) {
      _server->sendHeader("Location", String(_redirect_uri)+"callback");
      _server->send(302, "text/plain", "");
    }
  }else{
    _server->send(200, "test/html", "Something went wrong, please try again");
  }
}
void Spotify::server_on_refresh() {
  if (strcmp(_refresh_token, "") == 0 || !_refresh_token) {
    if (_server->args() == 0) {
      char page[900];
      char redirect_callback[150];
      snprintf(redirect_callback, sizeof(redirect_callback),"%scallback", _redirect_uri);
      snprintf(page,sizeof(page),_login_page, _client_id, redirect_callback);
      _server->send(200, "text/html", String(page));
    } else {
      if (_server->hasArg("code")) {
        strncpy(_auth_code, _server->arg("code").c_str(), sizeof(_auth_code));
        if(_debug_on){
          Serial.printf("Auth Code: %s\n", _auth_code);
        }
        if(get_refresh_token(_auth_code, strcat(_redirect_uri, "callback"))){
          char message[500];
          snprintf(message,sizeof(message), "Setup Complete, Refresh Token: %s <br>You can now close this page", _refresh_token);
          _server->send(200, "text/html", message);
        }
        else{
          _server->send(200, "text/html", "Something went wrong, please try again");
        }
        _server->stop();
      } else {
        char page[900];
        snprintf(page,sizeof(page),_login_page, _client_id, _redirect_uri);
        _server->send(200, "text/html", String(page));
      }
    }
  } else {
    _server->send(200, "text/html", "Spotify setup complete");
    _server->stop();
  }
}
void Spotify::server_routes(){
  _server->on("/", HTTP_GET, [this](){
    if(_debug_on){
      Serial.println("Send response to root");
    }
    auto root = callback_fn_root(this);
    root();
  });
  _server->on("/callback", HTTP_GET, [this](){
    if(_debug_on){
      Serial.println("Send response to Callback");
    }
    auto refresh = callback_fn_refresh(this);
    refresh();
  });
  
  _server->on("/get", HTTP_GET, [this](){
    if(_debug_on){
      Serial.println("Receive credentials from root");
    }
    auto response = callback_fn_response(this);
    response();
  });
  _server->begin();
  if(_debug_on){
    Serial.println("Server started");
  }
}
#endif
void Spotify::begin(){ 
  #ifndef DISABLE_WEB_SERVER
  if(!is_auth()){
    if(_port == 80){
      snprintf(_redirect_uri,sizeof(_redirect_uri), "http://%s/", WiFi.localIP().toString().c_str());
      Serial.printf("Go to this url in your Browser to login to spotify or enter your credentials: %s\n", _redirect_uri);
    }
    else{
      snprintf(_redirect_uri,sizeof(_redirect_uri), "http://%s:%d/", WiFi.localIP().toString().c_str(), _port);
      Serial.printf("Go to this url in your Browser to login to spotify or enter your credentials: %s\n", _redirect_uri);
    }
    server_routes();
  }
  #endif
  _client.setCACert(_spotify_root_ca);
  _client.setTimeout(10000);
}

bool Spotify::has_access_token(){
  return _access_token[0] == '\n';
}
bool Spotify::get_access_token(){
  return get_token();
}
void Spotify::handle_client(){
  #ifndef DISABLE_WEB_SERVER
  _server->handleClient(); 
  #endif
}
void Spotify::end(){
  delete this;
}
bool Spotify::is_auth(){
  return !(strcmp(_refresh_token, "") == 0 || !_refresh_token || strcmp(_client_id, "") == 0 || !_client_id || strcmp(_client_secret, "") == 0 || !_client_secret);
}
//Basic functions
response Spotify::RestApi(const char* rest_url, const char* type, int payload_size, const char* payload, JsonDocument filter){
  response response_obj;
  init_response(&response_obj);

  if (!_client.connect(_host, 443)) {
    deserializeJson(response_obj.reply ,"Connection failed");
    return response_obj;
  }
  _client.println(String(type) + " " + String(rest_url) + " HTTP/1.1");
  _client.println("Host: " + String(_host));
  _client.println("User-Agent: ESP32");
  _client.println("Connection: close");
  _client.println("Accept: application/json");
  
  _client.println("Authorization: Bearer " + String(_access_token));
  _client.println("Content-Type: application/json");
  if(strcmp(type, "GET") == 0){
    _client.println();
  }
  else if(strcmp(type, "PUT") == 0){
    _client.println("Content-Length: " + String(payload_size));
    _client.println();
    _client.println(payload);
  }
  else if(strcmp(type, "POST") == 0){
    _client.println("Content-Length: " + String(payload_size));
    _client.println();
    _client.println(payload);
  }
  else if(strcmp(type, "DELETE") == 0){
    _client.println();
    _client.println(payload);
  }

  header_resp header_data = process_headers();
  JsonDocument response = process_response(header_data, filter);
  if(_debug_on){
    Serial.printf("%s \"%s\" Status: %i\n", type, extract_endpoint(rest_url).c_str(), header_data.http_code);
    Serial.print("Reply: ");
    serializeJson(response, Serial);
    Serial.println();
  }
  
  if(_retry <= _max_num_retry && !valid_http_code(header_data.http_code)){
    _client.stop();
    String message = response["error"]["message"].as<String>();
    _retry++;
    if(message == "Only valid bearer authentication supported"){
      if(get_token()){
        return RestApi(rest_url, type, payload_size, payload, filter);
      }else{
        deserializeJson(response_obj.reply, "Failed to get token");
      }
    }
    else{
      deserializeJson(response_obj.reply, response["error"]["message"].as<String>());
    }
  }else{
    _client.stop();
    response_obj.reply = response;
  }
  
  response_obj.status_code = header_data.http_code;
  return response_obj;
}

response Spotify::RestApiPut(const char* rest_url,int payload_size, const char* payload){
  return RestApi(rest_url, "PUT", payload_size, payload);
}
response Spotify::RestApiGet(const char* rest_url, JsonDocument filter){
  return RestApi(rest_url, "GET", 0, "", filter);
}
response Spotify::RestApiPost(const char* rest_url,int payload_size, const char* payload){
  return RestApi(rest_url, "POST", payload_size, payload);
}
response Spotify::RestApiDelete(const char* rest_url, const char* payload){
  return RestApi(rest_url, "DELETE", 0, payload);
}

bool Spotify::token_base_req(String payload){
  if (!_client.connect(_token_host, 443)) {
    return false;
  }
  String authorization = String(_client_id) + ":" + String(_client_secret);
  authorization.trim();
  authorization = "Basic " + base64::encode(authorization);
  _client.println("POST /api/token HTTP/1.1");
  _client.println("Host: " + String(_token_host));
  _client.println("User-Agent: ESP32");
  _client.println("Connection: close");
  _client.println("Content-Type: application/x-www-form-urlencoded");
  _client.println("Authorization: " + authorization);
  _client.println("Content-Length: " + String(payload.length()));
  _client.println();
  _client.println(payload); 
  return true;
}
bool Spotify::get_refresh_token(const char* auth_code, const char* redirect_uri) {
  bool reply = false; 
  String payload = "grant_type=authorization_code&code=" + String(auth_code) + "&redirect_uri=" + String(redirect_uri);
  if(!token_base_req(payload)){
    _client.stop();
    return false;
  }
  header_resp header_data = process_headers();
  if(header_data.http_code == -1){
    _client.stop();
    return false;
  }
  JsonDocument filter;
  filter["refresh_token"] = true;
  JsonDocument response = process_response(header_data, filter);
  if(!response.isNull() && response.containsKey("refresh_token")){
    reply = true;
    strncpy(_refresh_token, response["refresh_token"].as<const char*>(), sizeof(_refresh_token));
  }
  if (_debug_on) {
    Serial.printf("POST \"refresh token\" Status: %d \n", header_data.http_code);
    Serial.print("Reply: ");
    serializeJson(response, Serial);
    Serial.println();
  }
  _client.stop();
  return reply;
}

bool Spotify::get_token() {
  bool reply = false;
  String payload = "grant_type=refresh_token&refresh_token=" + String(_refresh_token);
  if(!token_base_req(payload)){
    _client.stop();
    return false;
  }
  header_resp header_data = process_headers();
  if(header_data.http_code == -1){
    _client.stop();
    return false;
  }
  JsonDocument filter;
  filter["access_token"] = true;
  JsonDocument response = process_response(header_data, filter);
  if(!response.isNull() && response.containsKey("access_token")){
    reply = true;
    strncpy(_access_token, response["access_token"].as<const char*>(), sizeof(_access_token));
  }
  if (_debug_on) {
    Serial.printf("POST \"acces token\" Status: %d \n", header_data.http_code);
    Serial.print("Reply: ");
    serializeJson(response, Serial);
    Serial.println();
  }
  _client.stop();
  return reply;
}

bool Spotify::valid_http_code(int code){
  return (code >= 200 && code <= 299);
}

header_resp Spotify::process_headers(){
  header_resp response;
  response.http_code = -1;
  response.content_length = 0;
  bool can_break = false;
  while (_client.connected()) {
    String line = _client.readStringUntil('\n');
    if (line.startsWith("HTTP")) {
      response.http_code = line.substring(9, 12).toInt();
    } else if(line.startsWith("Content-Type") || line.startsWith("content-type")){
      response.content_type = line.substring(14);
    }else if (line.startsWith("Content-Length") || line.startsWith("content-length")) {
      response.content_length = line.substring(16).toInt();
      can_break = true;
    }
    if(_debug_on){
      if (line.startsWith("HTTP") || line.startsWith("Content-Length") || line.startsWith("Content-Type")|| line.startsWith("content-length") || line.startsWith("content-type")){
        Serial.println(line);
      }
    }else if(can_break){
      _client.find("\r\n\r\n");
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  return response;
}

JsonDocument Spotify::process_response(header_resp header_data, JsonDocument filter){
  JsonDocument response;
  size_t recv_bytes = 0;
  if(_debug_on){
    Serial.printf("Filter: %s\n", filter.isNull() ? "Off" : "On");
  }
  if(!_client.connected() || header_data.content_type.indexOf("application/json") == -1){
    String response_str = "";
    if(header_data.http_code == 204){
      response["message"] = "No Content";
    }else if(!_client.connected()){
      response["message"] = "Client got disconnected";
    }else{
      while (_client.connected() && recv_bytes < header_data.content_length){
        recv_bytes += _client.available();
        response_str += _client.readStringUntil('\n') + "\n";
      }
      response["message"] = response_str;
    }
    if(_debug_on){
      serializeJsonPretty(response, Serial);
    }
    return response;
  }
  while (recv_bytes < header_data.content_length){
    recv_bytes += _client.available();
    if (filter.isNull() || !valid_http_code(header_data.http_code)) {
      DeserializationError err = deserializeJson(response, _client);
      if(err && _debug_on){
        Serial.printf("Error: %s\n", err.c_str());
      }
      break;
    } else if(!filter.isNull()) {
      DeserializationError err = deserializeJson(response, _client, DeserializationOption::Filter(filter));
      if(err && _debug_on){
        Serial.printf("Error: %s\n", err.c_str());
      }
      break;
    }
  }
  return response;
}
void Spotify::init_response(response* response_obj){
  response_obj -> status_code = -1;
  deserializeJson(response_obj -> reply, "If you see this, something went wrong");
}

char* Spotify::array_to_char(int size, const char** array, char* result) {
  result[0] = '\0';
  for (int i = 0; i < size; ++i) {
    strcat(result, array[i]);
    if (i != size - 1) {
      strcat(result, ",");
    }
  }
  return result;
}
void Spotify::array_to_json_array(int size, const char** array, char* data, int data_size){
  JsonDocument doc;
  JsonArray json_array = doc.to<JsonArray>();
  for (int i = 0; i<size; ++i) {
    json_array.add(array[i]);
  }
  serializeJson(json_array,data,data_size);
}

String Spotify::extract_endpoint(const char* url) {
  String urlStr(url);
  int startPos = urlStr.indexOf("https://api.spotify.com/v1/");
  if (startPos >= 0) {
    startPos += String("https://api.spotify.com/v1/").length();
    int endPos = urlStr.indexOf('?', startPos);
    if (endPos >= 0) {
      return String(urlStr.substring(startPos, endPos).c_str());
    }else{
      return String(urlStr.substring(startPos).c_str());
    }
  }
  return "";
}
//API functions
//Player
#ifndef DISABLE_PLAYER
response Spotify::currently_playing(JsonDocument filter){
  char url[60];
  snprintf(url, sizeof(url),"%sme/player/currently-playing", _base_url);
  return RestApiGet(url, filter);
}
response Spotify::current_playback_state(JsonDocument filter){
  char url[60];
  snprintf(url, sizeof(url),"%sme/player", _base_url);
  return RestApiGet(url, filter);
}
response Spotify::start_resume_playback(const char* context_uri, int offset, int position_ms, const char* device_id){
  char url[100];
  snprintf(url, sizeof(url),"%sme/player/play", _base_url);
  if(device_id != nullptr){
    snprintf(url,sizeof(url), "%sme/player/play?device_id=%s",_base_url, device_id);
  }
  char payload[_max_char_size]; 
  int payload_size = 0;
  if(context_uri != nullptr){
    if (strncmp(context_uri, "spotify:track:", 14) == 0) {
      snprintf(payload,sizeof(payload), "{\"uris\":[\"%s\"]}", context_uri);
      
    } else {
      snprintf(payload,sizeof(payload), "{\"context_uri\":\"%s\",\"offset\":{\"position\":%d},\"position_ms\":%d}",context_uri, offset, position_ms);
    }
    payload_size = strlen(payload);
  }
  else{
    payload_size = 0;
    payload[0] = '\0';
  }

  return RestApiPut(url, payload_size, payload);
}
response Spotify::start_resume_playback(int size, const char** uris, const char* device_id){
  char url[100];
  snprintf(url, sizeof(url),"%sme/player/play", _base_url);
  if(device_id != nullptr){
    snprintf(url,sizeof(url), "%sme/player/play?device_id=%s",_base_url, device_id);
  }
  char payload[_max_char_size]; 
  int payload_size = 0;
  char arr[_max_char_size];
  array_to_json_array(size, uris, arr);
  snprintf(payload,sizeof(payload), "{\"uris\":%s}", arr);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::start_resume_playback(const char* device_id){
  char url[100];
  if(device_id != nullptr){
    snprintf(url, sizeof(url),"%sme/player/play?device_id=%s", _base_url, device_id);
  }
  else{
    snprintf(url, sizeof(url), "%sme/player/play", _base_url);
  }
  
  return RestApiPut(url);
}
response Spotify::pause_playback(){
  char url[50];
  snprintf(url, sizeof(url),"%sme/player/pause", _base_url);

  return RestApiPut(url);
}
response Spotify::skip(){
  char url[50];
  snprintf(url, sizeof(url),"%sme/player/next", _base_url);

  return RestApiPost(url); 
}
response Spotify::previous(){
  char url[50];
  snprintf(url, sizeof(url),"%sme/player/previous", _base_url);
  
  return RestApiPost(url);
}
response Spotify::available_devices(JsonDocument filter){
  char url[50];
  snprintf(url, sizeof(url),"%sme/player/devices", _base_url);

  return RestApiGet(url, filter);
}
response Spotify::recently_played_tracks(int limit, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sme/player/recently-played?limit=%d", _base_url, limit);
  return RestApiGet(url, filter);
}
response Spotify::get_queue(JsonDocument filter){
  char url[50];
  snprintf(url, sizeof(url),"%sme/player/queue", _base_url);

  return RestApiGet(url, filter);
}
response Spotify::transfer_playback(const char* device_id){
  char url[40];
  snprintf(url, sizeof(url),"%sme/player", _base_url);

  char payload[100];
  int payload_size = 0;
  snprintf(payload,sizeof(payload), "{\"device_ids\":[\"%s\"]}", device_id);
  payload_size = strlen(payload);
  return RestApiPut(url, payload_size, payload);
}
response Spotify::seek_to_position(int time_ms){
  char url[100];
  snprintf(url, sizeof(url), "%sme/player/seek?position_ms=%d", _base_url, time_ms);

  return RestApiPut(url);
}
response Spotify::repeat_mode(const char* mode){
  char url[100];
  snprintf(url, sizeof(url), "%sme/player/repeat?state=%s", _base_url, mode);

  return RestApiPut(url);
}
response Spotify::set_volume(int value){
  char url[100];
  snprintf(url, sizeof(url), "%sme/player/volume?volume_percent=%d", _base_url, value);

  return RestApiPut(url);
}
response Spotify::shuffle(bool mode){
  char state[6]; 
  snprintf(state,sizeof(state), "%s", mode ? "true" : "false");

  char url[100]; 
  snprintf(url, sizeof(url), "%sme/player/shuffle?state=%s", _base_url, state);

  return RestApiPut(url);
}
response Spotify::add_to_queue(const char* context_uri){
  char url[100];
  snprintf(url, sizeof(url), "%sme/player/queue?uri=%s", _base_url, context_uri);

  return RestApiPost(url);
}
#endif
#ifndef DISABLE_ALBUM
//Albums
response Spotify::get_album(const char* album_id, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%salbums/%s", _base_url, album_id);
  
  return RestApiGet(url, filter);
}
response Spotify::get_albums(int size, const char ** album_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%salbums?ids=%s", _base_url, array_to_char(size, album_ids,arr));
  
  return RestApiGet(url, filter);
}
response Spotify::get_album_tracks(const char* album_id, int limit, int offset, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%salbums/%s/tracks?limit=%d&offset=%d", _base_url, album_id, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::get_users_saved_albums(int limit, int offset, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sme/albums?limit=%d&offset=%d", _base_url, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::save_albums_for_current_user(int size, const char ** album_ids){
  char url[40];
  snprintf(url, sizeof(url), "%sme/albums", _base_url);
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, album_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_users_saved_albums(int size, const char ** album_ids){
  char url[40];
  snprintf(url, sizeof(url), "%sme/albums", _base_url);
  char payload[_max_char_size];
  array_to_json_array(size, album_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_useres_saved_albums(int size, const char ** album_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/albums/contains?ids=%s", _base_url, array_to_char(size, album_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_new_releases(int limit, int offset, const char* country, JsonDocument filter){
  char url[120];
  if(country == nullptr){
    snprintf(url, sizeof(url), "%sbrowse/new-releases?limit=%d&offset=%d", _base_url, limit, offset);
  }
  else{
    snprintf(url, sizeof(url), "%sbrowse/new-releases?country=%s&limit=%d&offset=%d", _base_url, country, limit, offset);
  }

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_ARTIST
//Artists
response Spotify::get_artist(const char* artist_id, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sartists/%s", _base_url, artist_id);

  return RestApiGet(url, filter);
}
response Spotify::get_several_artists(int size, const char ** artist_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sartists?ids=%s", _base_url, array_to_char(size, artist_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_artist_albums(const char* artist_id, int size_groups, const char** include_groups, int limit, int offset, JsonDocument filter){
  char url[200];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sartists/%s/albums?include_groups=%s&limit=%d&offset=%d", _base_url, artist_id, array_to_char(size_groups,include_groups, arr), limit, offset);  

  return RestApiGet(url, filter);
}
response Spotify::get_artist_top_tracks(const char* artist_id, const char* country, JsonDocument filter){
  char url[100];
  if(country == nullptr){
    snprintf(url, sizeof(url), "%sartists/%s/top-tracks", _base_url, artist_id);
  }
  else{
    snprintf(url, sizeof(url), "%sartists/%s/top-tracks?country=%s", _base_url, artist_id, country);
  }

  return RestApiGet(url, filter);
}
response Spotify::get_artist_related_artist(const char* artist_id, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sartists/%s/related-artists", _base_url, artist_id);

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_AUDIOBOOKS
//Audiobooks
response Spotify::get_audiobook(const char* audiobook_id, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%saudiobooks/%s", _base_url, audiobook_id);

  return RestApiGet(url, filter);
}
response Spotify::get_several_audiobooks(int size, const char** audiobook_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%saudiobooks?ids=%s", _base_url, array_to_char(size, audiobook_ids,arr));

  return RestApiGet(url, filter);
}
response Spotify::get_audiobook_chapters(const char* audiobook_id, int limit, int offset, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%saudiobooks/%s/chapters?limit=%d&offset=%d", _base_url, audiobook_id, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::get_users_audiobooks(int limit, int offset, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sme/audiobooks?limit=%d&offset=%d", _base_url, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::save_audiobooks_for_current_user(int size, const char** audiobook_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/audiobooks?ids=%s", _base_url, array_to_char(size, audiobook_ids, arr));

  return RestApiPut(url);
}
response Spotify::remove_audiobooks_for_current_user(int size, const char** audiobook_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/audiobooks?ids=%s", _base_url, array_to_char(size, audiobook_ids, arr));

  return RestApiDelete(url);
}
response Spotify::check_users_saved_audiobooks(int size, const char** audiobook_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/audiobooks/contains?ids=%s", _base_url, array_to_char(size, audiobook_ids,arr));

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_CATEGORIES
//Categories
response Spotify::get_several_browse_categories(int limit, int offset, const char* country, const char* locale, JsonDocument filter){
  char url[200];
  if((country == nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "%sbrowse/categories?limit=%d&offset=%d", _base_url, limit, offset);
  }
  else if((country == nullptr)&&(locale != nullptr)){
    snprintf(url, sizeof(url), "%sbrowse/categories?locale=%s&limit=%d&offset=%d", _base_url, locale, limit, offset);
  }
  else if((country != nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "%sbrowse/categories?country=%s&limit=%d&offset=%d", _base_url, country, limit, offset);
  }
  else{
    snprintf(url, sizeof(url), "%sbrowse/categories?country=%s&locale=%s&limit=%d&offset=%d", _base_url, country, locale, limit, offset);
  }

  return RestApiGet(url, filter);
}
response Spotify::get_single_browse_category(const char* category_id, const char* country, const char* locale, JsonDocument filter){
  char url[150];
  if((country == nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "%sbrowse/categories/%s", _base_url, category_id);
  }
  else if((country == nullptr)&&(locale != nullptr)){
    snprintf(url, sizeof(url), "%sbrowse/categories/%s?locale=%s", _base_url, category_id, locale);
  }
  else if((country != nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "%sbrowse/categories/%s?country=%s", _base_url, category_id, country);
  }
  else{
    snprintf(url, sizeof(url), "%sbrowse/categories/%s?country=%s&locale=%s", _base_url, category_id, country, locale);
  }

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_CHAPTERS
//Chapters
response Spotify::get_chapter(const char* chapter_id, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%schapters/%s", _base_url,  chapter_id);

  return RestApiGet(url, filter);
}
response Spotify::get_several_chapters(int size, const char** chapter_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%schapters?ids=%s", _base_url, array_to_char(size, chapter_ids, arr));

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_EPISODES
//Episodes
response Spotify::get_episode(const char* episode_id, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sepisodes/%s", _base_url, episode_id);

  return RestApiGet(url, filter);
}
response Spotify::get_several_episodes(int size, const char** episode_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sepisodes?ids=%s", _base_url, array_to_char(size, episode_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_users_saved_episodes(int limit, int offset, JsonDocument filter){
  char url[100];
  snprintf(url, sizeof(url), "%sme/episodes?limit=%d&offset=%d", _base_url, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::save_episodes_for_current_user(int size, const char** episode_ids){
  char url[40];
  snprintf(url, sizeof(url), "%sme/episodes", _base_url);
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, episode_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_episodes_for_current_user(int size, const char** episode_ids){
  char url[40];
  snprintf(url, sizeof(url), "%sme/episodes", _base_url);
  char payload[_max_char_size];
  array_to_json_array(size, episode_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_users_saved_episodes(int size, const char** episode_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/episodes/contains?ids=%s", _base_url, array_to_char(size, episode_ids, arr));

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_GENRES
//Genres
response Spotify::get_available_genre_seeds(JsonDocument filter){
  char url[70];
  snprintf(url, sizeof(url), "%srecommendations/available-genre-seeds", _base_url);

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_MARKETS
//Markets
response Spotify::get_available_markets(JsonDocument filter){
  char url[40];
  snprintf(url, sizeof(url), "%smarkets", _base_url);
  
  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_PLAYLISTS
//Playlist
response Spotify::get_playlist(const char* playlist_id, int size, const char** fields, int size_of_additional_types, const char** additional_types, JsonDocument filter) {
  char url[200];
  if(size == 0){
    if(size_of_additional_types == 0){
      snprintf(url, sizeof(url), "%splaylists/%s", _base_url, playlist_id);
    }
    else{
      char arr[_max_char_size];
      snprintf(url, sizeof(url), "%splaylists/%s?additional_types=%s", _base_url, playlist_id, array_to_char(size_of_additional_types, additional_types, arr));
    }
  }
  else{
    if(size_of_additional_types == 0){
      char arr[_max_char_size];
      snprintf(url, sizeof(url), "%splaylists/%s?fields=%s", _base_url, playlist_id, array_to_char(size, fields, arr));
    }
    else{
      char arr[_max_char_size];
      char arr2[_max_char_size];
      snprintf(url, sizeof(url), "%splaylists/%s?fields=%s&additional_types=%s", _base_url, playlist_id, array_to_char(size, fields, arr), array_to_char(size_of_additional_types, additional_types, arr2));
    }
  }

  return RestApiGet(url, filter);
}
response Spotify::change_playlist_details(const char* playlist_id, const char* name, bool is_public, bool is_collaborative, const char* description) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s", _base_url, playlist_id);

  JsonDocument doc;
  if (is_public && is_collaborative){
    is_collaborative = false;
  }
  doc["collaborative"] = is_collaborative;
  doc["name"] = name;
  doc["public"] = is_public;
  doc["description"] = description;

  char payload[200];
  int payload_size = 0;
  serializeJson(doc, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::get_playlist_items(const char* playlist_id, const char* fields, int limit, int offset, JsonDocument filter) {
  char url[200];
  snprintf(url, sizeof(url), "%splaylists/%s/tracks?fields=%s&limit=%d&offset=%d", _base_url, playlist_id, fields, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::update_playlist_items(const char* playlist_id, int size, const char** uris, int range_length, int range_start, int insert_before) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/tracks", _base_url, playlist_id);
  
  JsonDocument doc;
  char arr[_max_char_size];
  array_to_json_array(size, uris, arr);
  doc["uris"] = arr;
  doc["range_start"] = range_start;
  doc["insert_before"] = insert_before;
  doc["range_length"] = range_length;

  char payload[_max_char_size];
  int payload_size = 0;
  serializeJson(doc, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::add_items_to_playlist(const char* playlist_id, int size, const char ** uris, int position) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/tracks", _base_url,  playlist_id);
  
  JsonDocument doc;
  char arr[_max_char_size];
  array_to_json_array(size, uris, arr);
  doc["uris"] = arr;
  doc["position"] = position;

  char payload[_max_char_size];
  int payload_size = 0;
  serializeJson(doc, payload);
  payload_size = strlen(payload);

  return RestApiPost(url, payload_size, payload);
}
response Spotify::remove_playlist_items(const char* playlist_id, int size, const char** uris) { 
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/tracks", _base_url, playlist_id);
  char arr[_max_char_size];
  char* payload = array_to_char(size, uris, arr);
  
  
  return RestApiDelete(url, payload);
}
response Spotify::get_current_users_playlists(int limit, int offset, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/playlists?limit=%d&offset=%d", _base_url, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::get_user_playlists(const char* user_id, int limit, int offset, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%susers/%s/playlists?limit=%d&offset=%d", _base_url, user_id, limit, offset);
  
  return RestApiGet(url, filter);
}
response Spotify::create_playlist(const char* user_id, const  char* name, bool is_public, bool is_collaborative, const char* description) {
  char url[100];
  snprintf(url, sizeof(url), "%susers/%s/playlists", _base_url, user_id);

  if (is_public && is_collaborative){
    is_collaborative = false;
  }

  JsonDocument doc;
  doc["name"] = name;
  doc["public"] = is_public;
  doc["collaborative"] = is_collaborative;
  doc["description"] = description;
  char payload[256];
  int  payload_size = 0;
  serializeJson(doc, payload);
  payload_size = strlen(payload);

  return RestApiPost(url, payload_size, payload);
}
response Spotify::get_featured_playlists(int limit, int offset, const char* timestamp, const char* country, const char* locale, JsonDocument filter) {
    char url[200];
    if (timestamp) {
        if (!country && !locale) {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?timestamp=%s&limit=%d&offset=%d", _base_url, timestamp, limit, offset);
        }
        else if (!country && locale) {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?locale=%s&timestamp=%s&limit=%d&offset=%d", _base_url, locale, timestamp, limit, offset);
        }
        else if (country && !locale) {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?country=%s&timestamp=%s&limit=%d&offset=%d", _base_url, country, timestamp, limit, offset);
        }
        else {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?country=%s&locale=%s&timestamp=%s&limit=%d&offset=%d", _base_url, country, locale, timestamp, limit, offset);
        }
    }
    else {
        if (!country && !locale) {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?limit=%d&offset=%d", _base_url, limit, offset);
        }
        else if (!country && locale) {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?locale=%s&limit=%d&offset=%d", _base_url, locale, limit, offset);
        }
        else if (country && !locale) {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?country=%s&limit=%d&offset=%d", _base_url, country, limit, offset);
        }
        else {
            snprintf(url, sizeof(url), "%sbrowse/featured-playlists?country=%s&locale=%s&limit=%d&offset=%d", _base_url, country, locale, limit, offset);
        }
    }
    return RestApiGet(url, filter);
}
response Spotify::get_category_playlists(const char* category_id, int limit, int offset, const char* country, JsonDocument filter) {
  char url[200];
  if(country == nullptr){
    snprintf(url, sizeof(url), "%sbrowse/categories/%s/playlists?limit=%d&offset=%d", _base_url, category_id, limit, offset);
  }
  else{
    snprintf(url, sizeof(url), "%sbrowse/categories/%s/playlists?country=%s&limit=%d&offset=%d", _base_url, category_id, country, limit, offset);
  }

  return RestApiGet(url, filter);
}
response Spotify::get_playlist_cover_image(const char* playlist_id, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/images", _base_url, playlist_id);

  return RestApiGet(url, filter);
}
response Spotify::add_custom_playlist_cover_image(const char* playlist_id, const char* data) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/images", _base_url, playlist_id);  
  int payload_size = strlen(data);

  return RestApiPut(url, payload_size, data);
}
#endif
#ifndef DISABLE_SEARCH
//Search
response Spotify::search(const char* q, int type_size, const char** type, int limit, int offset, const char* market, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  if(market){
    if(type_size == 0){
      snprintf(url, sizeof(url), "%ssearch?q=%s&limit=%d&offset=%d&market=%s", _base_url, q, limit, offset, market);
    }
    else{
      snprintf(url, sizeof(url), "%ssearch?q=%s&type=%s&limit=%d&offset=%d&market=%s", _base_url, q, array_to_char(type_size, type, arr), limit, offset, market);
    }
    
  }
  else{
    if(type_size == 0){
      snprintf(url, sizeof(url), "%ssearch?q=%s&limit=%d&offset=%d", _base_url, q, limit, offset);
    }
    else{
      snprintf(url, sizeof(url), "%ssearch?q=%s&type=%s&limit=%d&offset=%d", _base_url, q, array_to_char(type_size, type, arr), limit, offset);
    }
  }

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_SHOWS
//Shows
response Spotify::get_show(const char* show_id, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%sshows/%s", _base_url, show_id);

  return RestApiGet(url, filter);
}
response Spotify::get_several_shows(int size, const char ** show_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sshows?ids=%s", _base_url, array_to_char(size, show_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_show_episodes(const char* show_id, int limit, int offset, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%sshows/%s/episodes?limit=%d&offset=%d", _base_url, show_id, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::get_users_saved_shows(int limit, int offset, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/shows?limit=%d&offset=%d", _base_url, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::save_shows_for_current_user(int size, const char ** show_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/shows?ids=%s", _base_url, array_to_char(size, show_ids, arr));

  return RestApiPut(url);
}
response Spotify::remove_shows_for_current_user(int size, const char ** show_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/shows?ids=%s", _base_url, array_to_char(size, show_ids, arr));

  return RestApiDelete(url);
}
response Spotify::check_users_saved_shows(int size, const char ** show_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/shows/contains?ids=%s", _base_url, array_to_char(size, show_ids, arr));

  return RestApiGet(url, filter);
}
#endif
#ifndef DISABLE_TRACKS
//Tracks
response Spotify::get_track(const char* track_id, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%stracks/%s", _base_url, track_id);

  return RestApiGet(url, filter);
}
response Spotify::get_several_tracks(int size, const char ** track_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%stracks?ids=%s", _base_url, array_to_char(size, track_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_user_saved_tracks(int limit, int offset, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/tracks?limit=%d&offset=%d", _base_url, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::save_tracks_for_current_user(int size, const char ** track_ids) {
  char url[40];
  snprintf(url, sizeof(url), "%sme/tracks", _base_url);
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, track_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_user_saved_tracks(int size, const char ** track_ids) {
  char url[40];
  snprintf(url, sizeof(url), "%sme/tracks", _base_url);
  char payload[_max_char_size];
  array_to_json_array(size, track_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_user_saved_tracks(int size, const char ** track_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/tracks/contains?ids=%s", _base_url, array_to_char(size, track_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_tracks_audio_features(int size, const char ** track_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%saudio-features?ids=%s", _base_url,array_to_char(size, track_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::get_track_audio_features(const char* track_id, JsonDocument filter) {
  char url[150];
  snprintf(url, sizeof(url), "%saudio-features/%s", _base_url,track_id);

  return RestApiGet(url, filter);
}
response Spotify::get_track_audio_analysis(const char* track_id, JsonDocument filter) {
  char url[150];
  snprintf(url, sizeof(url), "%saudio-analysis/%s", _base_url, track_id);
  return RestApiGet(url, filter);
}
response Spotify::get_recommendations(recommendations& recom, int limit, JsonDocument filter){
  char url[2000];
  snprintf(url, sizeof(url), "%srecommendations?limit=%d", _base_url, limit);

  std::map<const char*, char*> char_params;
  std::map<const char*, float> float_params;
  populate_float_values(float_params, recom);
  populate_char_values(char_params, recom); 

  for (const auto& param : char_params) {
    char value[100];
    snprintf(value, sizeof(value), "&%s=%s",param.first, param.second);
    strcat(url, value);
  }
  for(const auto& param : float_params){
    char value[100];
    snprintf(value, sizeof(value), "&%s=%f",param.first, param.second);
    strcat(url, value);
  }
  return RestApiGet(url, filter);
}
bool Spotify::is_valid_value(float param) {
  return param >= 0.0 && param <= 1.0;
}
bool Spotify::is_valid_value(int param) {
  return param >0;
}
void Spotify::populate_float_values(std::map<const char*, float>& float_params, recommendations& recom){
  if (is_valid_value(recom.min_acousticness)) {
    float_params["min_acousticness"] = recom.min_acousticness;
  }
  if (is_valid_value(recom.max_acousticness)) {
    float_params["max_acousticness"] = recom.max_acousticness;
  }
  if (is_valid_value(recom.target_acousticness)) {
    float_params["target_acousticness"] = recom.target_acousticness;
  }

  if (is_valid_value(recom.min_danceability)) {
    float_params["min_danceability"] = recom.min_danceability;
  }
  if (is_valid_value(recom.max_danceability)) {
    float_params["max_danceability"] = recom.max_danceability;
  }
  if (is_valid_value(recom.target_danceability)) {
    float_params["target_danceability"] = recom.target_danceability;
  }

  if (is_valid_value(recom.min_duration_ms)) {
    float_params["min_duration_ms"] = recom.min_duration_ms;
  }
  if (is_valid_value(recom.max_duration_ms)) {
    float_params["max_duration_ms"] = recom.max_duration_ms;
  }
  if (is_valid_value(recom.target_duration_ms)) {
    float_params["target_duration_ms"] = recom.target_duration_ms;
  }

  if (is_valid_value(recom.min_energy)) {
    float_params["min_energy"] = recom.min_energy;
  }
  if (is_valid_value(recom.max_energy)) {
    float_params["max_energy"] = recom.max_energy;
  }
  if (is_valid_value(recom.target_energy)) {
    float_params["target_energy"] = recom.target_energy;
  }

  if (is_valid_value(recom.min_instrumentalness)) {
    float_params["min_instrumentalness"] = recom.min_instrumentalness;
  }
  if (is_valid_value(recom.max_instrumentalness)) {
    float_params["max_instrumentalness"] = recom.max_instrumentalness;
  }
  if (is_valid_value(recom.target_instrumentalness)) {
    float_params["target_instrumentalness"] = recom.target_instrumentalness;
  }

  if (is_valid_value(recom.min_key)) {
    float_params["min_key"] = recom.min_key;
  }
  if (is_valid_value(recom.max_key)) {
    float_params["max_key"] = recom.max_key;
  }
  if (is_valid_value(recom.target_key)) {
    float_params["target_key"] = recom.target_key;
  }

  if (is_valid_value(recom.min_liveness)) {
    float_params["min_liveness"] = recom.min_liveness;
  }
  if (is_valid_value(recom.max_liveness)) {
    float_params["max_liveness"] = recom.max_liveness;
  }
  if (is_valid_value(recom.target_liveness)) {
    float_params["target_liveness"] = recom.target_liveness;
  }

  if (is_valid_value(recom.min_loudness)) {
    float_params["min_loudness"] = recom.min_loudness;
  }
  if (is_valid_value(recom.max_loudness)) {
    float_params["max_loudness"] = recom.max_loudness;
  }
  if (is_valid_value(recom.target_loudness)) {
    float_params["target_loudness"] = recom.target_loudness;
  }

  if (is_valid_value(recom.min_mode)) {
    float_params["min_mode"] = recom.min_mode;
  }
  if (is_valid_value(recom.max_mode)) {
    float_params["max_mode"] = recom.max_mode;
  }
  if (is_valid_value(recom.target_mode)) {
    float_params["target_mode"] = recom.target_mode;
  }

  if (is_valid_value(recom.min_popularity)) {
    float_params["min_popularity"] = recom.min_popularity;
  }
  if (is_valid_value(recom.max_popularity)) {
    float_params["max_popularity"] = recom.max_popularity;
  }
  if (is_valid_value(recom.target_popularity)) {
    float_params["target_popularity"] = recom.target_popularity;
  }

  if (is_valid_value(recom.min_speechiness)) {
    float_params["min_speechiness"] = recom.min_speechiness;
  }
  if (is_valid_value(recom.max_speechiness)) {
    float_params["max_speechiness"] = recom.max_speechiness;
  }
  if (is_valid_value(recom.target_speechiness)) {
    float_params["target_speechiness"] = recom.target_speechiness;
  }

  if (is_valid_value(recom.min_tempo)) {
    float_params["min_tempo"] = recom.min_tempo;
  }
  if (is_valid_value(recom.max_tempo)) {
    float_params["max_tempo"] = recom.max_tempo;
  }
  if (is_valid_value(recom.target_tempo)) {
    float_params["target_tempo"] = recom.target_tempo;
  }

  if (is_valid_value(recom.min_time_signature)) {
    float_params["min_time_signature"] = recom.min_time_signature;
  }
  if (is_valid_value(recom.max_time_signature)) {
    float_params["max_time_signature"] = recom.max_time_signature;
  }
  if (is_valid_value(recom.target_time_signature)) {
    float_params["target_time_signature"] = recom.target_time_signature;
  }

  if (is_valid_value(recom.min_valence)) {
    float_params["min_valence"] = recom.min_valence;
  }
  if (is_valid_value(recom.max_valence)) {
    float_params["max_valence"] = recom.max_valence;
  }
  if (is_valid_value(recom.target_valence)) {
    float_params["target_valence"] = recom.target_valence;
  }
}
void Spotify::populate_char_values(std::map<const char*, char*>& char_params, recommendations& recom){
  char arr[_max_char_size];
  if(is_valid_value(recom.seed_artists_size)){
    char_params["seed_artists"] = array_to_char(recom.seed_artists_size, recom.seed_artists, arr);
  }
  if(is_valid_value(recom.seed_genres_size)){
    char_params["seed_genres"] = array_to_char(recom.seed_genres_size, recom.seed_genres, arr);
  }
  if(is_valid_value(recom.seed_tracks_size)){
    char_params["seed_tracks"] = array_to_char(recom.seed_tracks_size, recom.seed_tracks,arr);
  }
}
#endif
#ifndef DISABLE_USER
//Users
response Spotify::get_current_user_profile(JsonDocument filter) {
  char url[40];
  snprintf(url, sizeof(url), "%sme", _base_url);

  return RestApiGet(url, filter);
}
response Spotify::get_user_top_items(const char* type, const char* time_range, int limit, int offset, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/top/%s?time_range=%s&limit=%d&offset=%d", _base_url, type, time_range, limit, offset);

  return RestApiGet(url, filter);
}
response Spotify::get_user_profile(const char* user_id, JsonDocument filter) {
  char url[100];
  snprintf(url, sizeof(url), "%susers/%s", _base_url, user_id);

  return RestApiGet(url, filter);
}
response Spotify::follow_playlist(const char* playlist_id, bool is_public) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/followers", _base_url, playlist_id);
  char payload[100];
  int payload_size = 0;
  snprintf(payload,sizeof(payload), "{\"public\":%s}", is_public ? "true" : "false");
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_playlist(const char* playlist_id) {
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/followers", _base_url, playlist_id);

  return RestApiDelete(url);
}
response Spotify::get_followed_artists(const char* after, const char* type, int limit, JsonDocument filter) {
  char url[100];
  if(after == nullptr){
    snprintf(url, sizeof(url), "%sme/following?type=%s&limit=%d", _base_url, type, limit);
  }
  else{
    snprintf(url, sizeof(url), "%sme/following?type=%s&after=%s&limit=%d", _base_url, type, after, limit);
  }


  return RestApiGet(url, filter);
}
response Spotify::follow_artists_or_users(const char* type,int size, const char** artist_user_ids) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/following?type=%s", _base_url, type);
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, artist_user_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_artists_or_users(const char* type, int size, const char** artist_user_ids) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/following?type=%s", _base_url, type);
  char payload[_max_char_size];
  array_to_json_array(size, artist_user_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_if_user_follows_artists_or_users(const char* type, int size, const char** artist_user_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/following/contains?type=%s&ids=%s", _base_url, type, array_to_char(size, artist_user_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::check_if_users_follow_playlist(const char* playlist_id, int size, const char** user_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%splaylists/%s/followers/contains?ids=%s", _base_url, playlist_id, array_to_char(size, user_ids, arr));

  return RestApiGet(url, filter);
}
#endif
//Simplified functions, formatting functions
#ifndef DISABLE_SIMPLIFIED
String Spotify::current_track_name(){
  String track_name = "Something went wrong";
  JsonDocument filter;
  filter["item"]["name"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    track_name = data.reply["item"]["name"].as<String>();
  }
  return track_name;
}
String Spotify::current_track_id(){
  String track_id = "Something went wrong";
  JsonDocument filter;
  filter["item"]["id"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    track_id = data.reply["item"]["id"].as<String>();
  }
  return track_id;
}
String Spotify::current_device_id(){
  String device_id = "Something went wrong";
  JsonDocument filter;
  JsonObject obj = filter["devices"].add<JsonObject>();
  obj["id"] = true;
  obj["is_active"] = true;
  response data = available_devices(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    JsonArray devices = data.reply["devices"].as<JsonArray>();
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
  JsonDocument filter;
  filter["item"]["artists"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    JsonArray array = data.reply["item"]["artists"];
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

char* Spotify::current_device_id(char * device_id){
  JsonDocument filter;
  JsonObject obj = filter["devices"].add<JsonObject>();
  obj["id"] = true;
  obj["is_active"] = true;
  response data = available_devices(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    JsonArray devices = data.reply["devices"].as<JsonArray>();
    for (JsonVariant device : devices) {
      JsonObject deviceObj = device.as<JsonObject>();
      if (deviceObj["is_active"].as<bool>()) {
        strcpy(device_id, deviceObj["id"].as<String>().c_str());
        break;
      }
    }  
  }
  return device_id;
}
char* Spotify::current_track_name(char * track_name){
  JsonDocument filter;
  filter["item"]["name"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    strcpy(track_name, data.reply["item"]["name"].as<String>().c_str());
  }
  return track_name;
}
char* Spotify::current_track_id(char * track_id){
  JsonDocument filter;
  filter["item"]["id"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    strcpy(track_id, data.reply["item"]["id"].as<String>().c_str());
  }
  return track_id;
}
char* Spotify::current_artist_names(char * artist_names){
  JsonDocument filter;
  filter["item"]["artists"] = true;
  response data = currently_playing();
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    JsonArray array = data.reply["item"]["artists"];
    int len = array.size();
    artist_names[0] = '\0';
    for (int i = 0; i<len; i++){
      strcat(artist_names, array[i]["name"].as<String>().c_str());
      if (i<len-1){
        strcat(artist_names, ", ");
      }
    }
  }
  return artist_names;
}
bool Spotify::is_playing(){
  bool is_playing = true;
  JsonDocument filter;
  filter["is_playing"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    is_playing = data.reply["is_playing"].as<bool>();
  }
  return is_playing;
}
bool Spotify::volume_modifyable(){
  bool volume_modifyable = false;
  JsonDocument filter;
  filter["device"]["supports_volume"] = true;
  response data = current_playback_state(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    volume_modifyable = data.reply["device"]["supports_volume"];
  }
  return volume_modifyable;
}
#endif
char Spotify::convert_id_to_uri(const char* id, const char* type){
  char uri[_size_of_uri];
  snprintf(uri,_size_of_uri, "spotify:%s:%s", type, id);
  return *uri;
}
char* Spotify::convert_id_to_uri(const char* id, const char* type,char * uri){
  snprintf(uri,sizeof(uri), "spotify:%s:%s", type, id);
  return uri;
}
user_tokens Spotify::get_user_tokens(){
  user_tokens tokens;
  tokens.client_id = strdup(_client_id);
  tokens.client_secret = strdup(_client_secret);
  tokens.refresh_token = strdup(_refresh_token);
  return tokens;
}
void print_response(response response_obj){
  Serial.printf("Status: %d\n", response_obj.status_code);
  Serial.print("Reply: "); 
  serializeJsonPretty(response_obj.reply, Serial);
  Serial.println();
}
