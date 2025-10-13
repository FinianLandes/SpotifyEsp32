#include "SpotifyEsp32.h"

spotify_log_level_t _spotify_log_level = SPOTIFY_LOG_INFO;

namespace spotify_types {
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

Spotify::Spotify(const char* client_id, const char* client_secret, int max_num_retry){
  _spotify_log_level = SPOTIFY_LOG_NONE;
  _retry = 0;
  if(!(client_id && client_secret)){
    _no_credentials = true;
  }else{
    strncpy(_client_id, client_id, sizeof(_client_id));
    strncpy(_client_secret, client_secret, sizeof(_client_secret));
  }
  if(max_num_retry > 0){
    _max_num_retry = max_num_retry;
  }
  else{
    _max_num_retry = 1;
  }
}
Spotify::Spotify(const char* client_id, const char* client_secret, const char* refresh_token, int max_num_retry){
  _spotify_log_level = SPOTIFY_LOG_NONE;
  _retry = 0;
  if(!(client_id && client_secret && refresh_token)){
    _no_credentials = true;
  }else{
    strncpy(_client_id, client_id, sizeof(_client_id));
    strncpy(_client_secret, client_secret, sizeof(_client_secret));
    strncpy(_refresh_token, refresh_token, sizeof(_refresh_token));
  }
  if(max_num_retry > 0){
    _max_num_retry = max_num_retry;
  }
  else{
    _max_num_retry = 1;
  }
}
Spotify::~Spotify(){
  SPOTIFY_LOGI(_TAG, "Object Destructed");
}
void Spotify::end(){
  delete this;
}

void Spotify::set_log_level(spotify_log_level_t spotify_log_level){
  _spotify_log_level = spotify_log_level;
  SPOTIFY_LOGI(_TAG, "Spotify logging set to (level %d)", spotify_log_level);
}
void Spotify::begin() {
  SPOTIFY_LOGI(_TAG, "Initializing Spotify client...");
  if (!is_auth()) {
    get_random_state(_random_state, _state_len);
    SPOTIFY_LOGD(_TAG, "Generated random state: %s", _random_state);

    const char* scopes;
    if (_custom_scopes && strlen(_custom_scopes) > 0) {
      scopes = _custom_scopes;
      SPOTIFY_LOGD(_TAG, "Using custom scopes");
    } else {
      scopes = "ugc-image-upload user-read-playback-state user-modify-playback-state user-read-currently-playing app-remote-control streaming playlist-read-private playlist-read-collaborative playlist-modify-private playlist-modify-public user-follow-modify user-follow-read user-read-playback-position user-top-read user-read-recently-played user-library-read user-library-modify user-read-email user-read-private";
      SPOTIFY_LOGD(_TAG, "Using default scopes: all");
    }

    char url[1024];
    snprintf(url, sizeof(url),"https://accounts.spotify.com/authorize" "?client_id=%s" "&response_type=code" "&redirect_uri=%s" "&scope=%s" "&state=%s", _client_id, _redirect_uri, scopes,_random_state);

    Serial.println("Open this URL in your browser to authorize:");
    Serial.println(url);
  }
  SPOTIFY_LOGD(_TAG, "Setting Spotify CA certificate & Timeout");
  _client.setCACert(_spotify_root_ca);
  _client.setTimeout(_timeout);
  SPOTIFY_LOGI(_TAG, "Spotify client initialized.");
  if (is_auth()){
    get_token();
  }
}
void Spotify::handle_client() {
  static unsigned long start_time = 0;
  static bool connecting = false;
  static size_t pos = 0;
  static bool headers_parsed = false;
  static size_t content_length = 0;
  static char buffer[2048];

  if (!connecting) {
    SPOTIFY_LOGD(_TAG, "Starting connection to host: %s", _auth_host);

    _client.setInsecure();
    if (!_client.connect(_auth_host, 443)) {
      SPOTIFY_LOGE(_TAG, "Connection to host failed!");
      return;
    }

    char path[128];
    snprintf(path, sizeof(path), "/api/spotify/get_code?state=%s", _random_state);
    _client.print("GET ");
    _client.print(path);
    _client.print(" HTTP/1.1\r\nHost: ");
    _client.print(_auth_host);
    _client.print("\r\nConnection: close\r\n\r\n");

    connecting = true;
    start_time = millis();
    pos = 0;
    headers_parsed = false;
    content_length = 0;
    return;
  }

  if (millis() - start_time > _timeout) {
    SPOTIFY_LOGD(_TAG, "Timeout reached, stopping client.");
    _client.stop();
    _client.setCACert(_spotify_root_ca);
    connecting = false;
    return;
  }

  while (_client.available() && pos < sizeof(buffer) - 1) {
    buffer[pos++] = _client.read();
  }

  if (!_client.connected() && pos > 0) {
    buffer[pos] = '\0';

    char* body_start = strstr(buffer, "\r\n\r\n");
    if (!body_start) {
      SPOTIFY_LOGE(_TAG, "Body start couldn't be found, stopping client.");
      _client.stop();
      _client.setCACert(_spotify_root_ca);
      connecting = false;
      return;
    }
    body_start += 4;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body_start);
    if (err) {
      SPOTIFY_LOGE(_TAG, "Deserialization failed: %s", err.c_str());
    } else {
      const char* code = doc["code"];
      if (code && strlen(code) > 0) {
        strncpy(_auth_code, code, sizeof(_auth_code)-1);
        _auth_code[sizeof(_auth_code)-1] = '\0';
        SPOTIFY_LOGD(_TAG, "Auth code received: %s", _auth_code);
        get_refresh_token(_auth_code, _redirect_uri);
        if (is_auth()){
          get_token();
        }
      } else {
        SPOTIFY_LOGD(_TAG, "Code was empty or null");
      }
    }
    _client.stop();
    _client.setCACert(_spotify_root_ca);
    connecting = false;
  }
}

void Spotify::set_scopes(const char* scopes) {
  _custom_scopes = scopes;
}
void Spotify::get_random_state(char* state, size_t len){
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const char prefix[] = "ESP32-";
  size_t prefix_len = strlen(prefix);
  strcpy(state, prefix);
  size_t max_random = len - prefix_len;
  for (size_t i = 0; i < max_random; i++) {
    int key = esp_random() % (sizeof(charset) - 1);
    state[prefix_len + i] = charset[key];
  }
  state[prefix_len + max_random] = '\0';
}

bool Spotify::has_access_token(){
  return _access_token[0] == '\n';
}
bool Spotify::get_access_token(){
  return get_token();
}
bool Spotify::is_auth(){
  return !(strcmp(_refresh_token, "") == 0 || !_refresh_token || strcmp(_client_id, "") == 0 || !_client_id);
}
//Basic functions
response Spotify::RestApi(const char* rest_url, const char* type, int payload_size, const char* payload, JsonDocument filter){
  response response_obj;
  init_response(&response_obj);

  _client.stop();
  if (!_client.connect(_host, 443)) {
    SPOTIFY_LOGE(_TAG, "Connection failed");
    deserializeJson(response_obj.reply ,"Connection failed");
    return response_obj;
  }
  SPOTIFY_LOGV(_TAG, "Creating %s request to %s", type, rest_url);
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
  else if((strcmp(type, "PUT") == 0 || strcmp(type, "POST") == 0 || strcmp(type, "DELETE") == 0)){
    SPOTIFY_LOGV(_TAG, "Sending payload %s with size %i ", payload ? payload: "NULL", payload_size);
    _client.println("Content-Length: " + String(payload_size));
    _client.println();
    if (payload_size > 0 && payload != nullptr){
      if (_client.write((const uint8_t*)payload, payload_size) != payload_size) {
        SPOTIFY_LOGE(_TAG, "Failed to send payload");
        deserializeJson(response_obj.reply, "Payload send failed");
        response_obj.status_code = -1;
        _client.stop();
        return response_obj;
      }
    }
  }
  _client.flush();
  header_resp header_data = process_headers();
  response_obj.status_code = header_data.http_code;
  SPOTIFY_LOGV(_TAG, "Header Data: Code %i, Content of length %i and type %s, Error: %s", header_data.http_code, header_data.content_length, header_data.content_type.c_str(), header_data.error.c_str());

  String str_response;
  JsonDocument response;
  if (!valid_http_code(header_data.http_code)){
    response = process_response(header_data);
  }else{
    response = process_response(header_data, filter);
  }
  serializeJson(response, str_response);

  SPOTIFY_LOGD(_TAG, "%s \"%s\" Status: %i", type, extract_endpoint(rest_url).c_str(), header_data.http_code);
  SPOTIFY_LOGD(_TAG, "Reply: %s", str_response.c_str());

  if(_retry <= _max_num_retry && !valid_http_code(header_data.http_code)){
    SPOTIFY_LOGD(_TAG, "Retrying");
    String message = response["error"]["message"].as<String>();
    _retry++;
    if(message == "Only valid bearer authentication supported"){
      _client.stop();
      SPOTIFY_LOGV(_TAG, "Trying to get new access token");
      if(get_token()){
        return RestApi(rest_url, type, payload_size, payload, filter);
      }else{
        SPOTIFY_LOGE(_TAG, "Failed to get new access token");
        deserializeJson(response_obj.reply, "Failed to get token");
        response_obj.status_code = -1;
      }
    }
    else{
      response_obj.reply = response;
    }
  }else{
    response_obj.reply = response;
  }
  _client.stop();
  _retry = 0;
  return response_obj;
}
response Spotify::RestApiPut(const char* rest_url, int payload_size, const char* payload){
  return RestApi(rest_url, "PUT", payload_size, payload);
}
response Spotify::RestApiGet(const char* rest_url, JsonDocument filter){
  return RestApi(rest_url, "GET", 0, "", filter);
}
response Spotify::RestApiPost(const char* rest_url, int payload_size, const char* payload){
  return RestApi(rest_url, "POST", payload_size, payload);
}
response Spotify::RestApiDelete(const char* rest_url, int payload_size, const char* payload){
  return RestApi(rest_url, "DELETE", payload_size, payload);
}

bool Spotify::token_base_req(const char* payload, size_t payload_len){
  if (!_client.connect(_token_host, 443)) {
    SPOTIFY_LOGE(_TAG, "Faild to connect to host for token request");
    return false;
  }
  char auth_raw[128];
  snprintf(auth_raw, sizeof(auth_raw), "%s:%s", _client_id, _client_secret);
  char auth_base64[192];
  size_t out_len = 0;
  int ret = mbedtls_base64_encode((unsigned char*)auth_base64, sizeof(auth_base64), &out_len, (const unsigned char*)auth_raw, strlen(auth_raw));
  auth_base64[strlen(auth_base64)] = '\0';

  _client.println("POST /api/token HTTP/1.1");
  _client.println("Host: " + String(_token_host));
  _client.println("User-Agent: ESP32");
  _client.println("Connection: close");
  _client.println("Content-Type: application/x-www-form-urlencoded");
  _client.print("Authorization: Basic ");
  _client.println(auth_base64);
  _client.print("Content-Length: ");
  _client.println((int)payload_len);
  _client.println();
  _client.write((const uint8_t*)payload, payload_len);
  _client.println();
  _client.flush();
  return true;
}
bool Spotify::get_refresh_token(const char* auth_code, const char* redirect_uri) {
  bool reply = false;
  char payload[1024];
  snprintf(payload, sizeof(payload), "grant_type=authorization_code&code=%s&redirect_uri=%s", auth_code, redirect_uri);

  if (!token_base_req(payload, strlen(payload))) {
    SPOTIFY_LOGE(_TAG, "Refresh token connection failed");
    _client.stop();
    return false;
  }

  header_resp header_data = process_headers();
  if (header_data.http_code == -1) {
    _client.stop();
    SPOTIFY_LOGE(_TAG, "Refresh token header processing failed");
    return false;
  }

  JsonDocument filter;
  filter["refresh_token"] = true;
  JsonDocument response = process_response(header_data, filter);

  if (!response.isNull() && !response["refresh_token"].isNull()) {
    reply = true;
    strncpy(_refresh_token, response["refresh_token"].as<const char*>(), sizeof(_refresh_token));
    _refresh_token[sizeof(_refresh_token) - 1] = '\0';
  }
  String str_response;
  serializeJson(response, str_response);
  SPOTIFY_LOGD(_TAG, "POST \"refresh token\" Status: %i", header_data.http_code);
  SPOTIFY_LOGD(_TAG, "Reply: %s", str_response.c_str());

  _client.stop();
  return reply;
}
bool Spotify::get_token() {
  bool reply = false;
  char payload[860];
  int written = snprintf(payload, sizeof(payload), "grant_type=refresh_token&refresh_token=%s", _refresh_token);
  if (written < 0 || written >= (int)sizeof(payload)) {
    return false;
  }

  if (!token_base_req(payload, strlen(payload))) {    
    SPOTIFY_LOGE(_TAG, "Access token connection failed");
    _client.stop();
    return false;
  }
  header_resp header_data = process_headers();
  if(header_data.http_code == -1){
    SPOTIFY_LOGE(_TAG, "Access token invalid or empty headers received");
    _client.stop();
    return false;
  }
  JsonDocument filter;
  filter["access_token"] = true;
  JsonDocument response = process_response(header_data, filter);
  if(!response.isNull() && !response["access_token"].isNull()){
    reply = true;
    strncpy(_access_token, response["access_token"].as<const char*>(), sizeof(_access_token));
  }
  String str_response;
  serializeJson(response, str_response);
  SPOTIFY_LOGD(_TAG, "POST \"access token\" Status: %i", header_data.http_code);
  SPOTIFY_LOGD(_TAG, "Reply: %s", str_response.c_str());
  _client.stop();
  return reply;
}

bool Spotify::valid_http_code(int code){
  return (code >= 200 && code <= 299);
}
header_resp Spotify::process_headers() {
  header_resp response;
  response.http_code = -1;
  response.content_length = 0;
  response.error = "";
  bool can_break = false;
  unsigned long last_data_receive = millis();
  while (_client.connected()) {
    String line = _client.readStringUntil('\n');
    if (line != "") {
      last_data_receive = millis();
    } 
    else if (line == "" && millis() - last_data_receive > _timeout) {
      response.http_code = -1;
      response.error = "Timeout receiving headers";
      SPOTIFY_LOGE(_TAG, "Timeout while receiving headers");
      break;
    }
    SPOTIFY_LOGV(_TAG, "Header: %s", line.c_str());

    // Parse HTTP response code
    if (line.startsWith("HTTP")) {
      response.http_code = line.substring(9, 12).toInt();
    } 
    // Parse Content-Type
    else if(line.startsWith("Content-Type") || line.startsWith("content-type")){
      response.content_type = line.substring(14);
    }
    // Parse Content-Length
    else if (line.startsWith("Content-Length") || line.startsWith("content-length")) {
      response.content_length = line.substring(16).toInt();
      can_break = true;
    }
    // If can_break is true, find the end of headers and break
    else if(can_break){
      _client.find("\r\n\r\n");
      break;
    }
    // If line is a carriage return, indicating end of headers, break
    if (line == "\r") {
      break;
    }
  }

  // If http_code is -1 and no error has been set, set a default error
  if(response.http_code == -1 && response.error == ""){
    response.error = "Client disconnected while receiving headers";
  }

  return response;
}
JsonDocument Spotify::process_response(header_resp header_data, JsonDocument filter) {
  JsonDocument response;
  SPOTIFY_LOGD(_TAG, "Filter: %s", filter.isNull() ? "Off" : "On");

  if (header_data.http_code == 204 || header_data.content_length == 0) {
    response["message"] = "No Content";
    return response;
  }

  if (header_data.http_code == -1) {
    response["message"] = header_data.error.isEmpty() ? "Client disconnected" : header_data.error;
    return response;
  }

  String raw_response = "";
  size_t bytes_read = 0;
  unsigned long start = millis();

  while (bytes_read < header_data.content_length && millis() - start < _timeout) {
    if (_client.available()) {
      char buf[128];
      int len = _client.readBytes(buf, min(sizeof(buf), header_data.content_length - bytes_read));
      if (len > 0) {
        raw_response += String(buf, len);
        bytes_read += len;
      } else if (len == 0) {
        SPOTIFY_LOGV(_TAG, "No more data available, stopping read");
        break;
      }
    }
  }

  SPOTIFY_LOGV(_TAG, "Raw response body: '%s' (read %d/%d bytes)", raw_response.c_str(), bytes_read, header_data.content_length);

  if (header_data.content_type.indexOf("application/json") != -1 && raw_response.length() > 0) {
    DeserializationError err;
    JsonDocument effective_filter;
    if (!filter.isNull()) {
      effective_filter = filter;
      effective_filter["error"] = true;
    }
    if (effective_filter.isNull()) {
      err = deserializeJson(response, raw_response);
    } else {
      err = deserializeJson(response, raw_response, DeserializationOption::Filter(effective_filter));
    }
    if (err) {
      SPOTIFY_LOGE(_TAG, "Error parsing JSON: %s", err.c_str());
      response["message"] = String("JSON parsing failed: ") + err.c_str();
    }
  } else {
    response["message"] = raw_response.length() ? raw_response : "No Body";
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
void Spotify::array_to_json_array_key(int size, const char** array, char* data, char* key, int data_size){
  JsonDocument doc;
  JsonArray json_array = doc[key].to<JsonArray>();
  for (int i = 0; i<size; ++i) {
    json_array.add(array[i]);
  }
  serializeJson(doc, data, data_size);
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
  char key[] = "ids";
  int payload_size = 0;
  array_to_json_array_key(size, album_ids, payload, key);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_users_saved_albums(int size, const char ** album_ids){
  char url[40];
  snprintf(url, sizeof(url), "%sme/albums", _base_url);
  char payload[_max_char_size];
  char key[] = "ids";
  int payload_size = 0;
  array_to_json_array_key(size, album_ids, payload, key);
  payload_size = strlen(payload);
  return RestApiDelete(url, payload_size, payload);
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
  char key[] = "ids";
  int payload_size = 0;
  array_to_json_array_key(size, episode_ids, payload, key);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_episodes_for_current_user(int size, const char** episode_ids){
  char url[40];
  snprintf(url, sizeof(url), "%sme/episodes", _base_url);
  char payload[_max_char_size];
  char key[] = "ids";
  int payload_size = 0;
  array_to_json_array_key(size, episode_ids, payload, key);
  payload_size = strlen(payload);
  return RestApiDelete(url, payload_size, payload);
}
response Spotify::check_users_saved_episodes(int size, const char** episode_ids, JsonDocument filter){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/episodes/contains?ids=%s", _base_url, array_to_char(size, episode_ids, arr));

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
  char data[_max_char_size];
  int data_size = 0;
  JsonDocument doc;
  JsonArray json_array = doc["uris"].to<JsonArray>();
  for (int i = 0; i<size; ++i) {
    json_array.add(uris[i]);
  }
  doc["position"] = position;
  serializeJson(doc, data);
  data_size = strlen(data);
  serializeJson(doc, data, data_size);

  char payload[_max_char_size];
  int payload_size = 0;
  serializeJson(doc, payload);
  payload_size = strlen(payload);
  return RestApiPost(url, payload_size, payload);
}
response Spotify::remove_playlist_items(const char* playlist_id, int size, const char** uris) { 
  char url[100];
  snprintf(url, sizeof(url), "%splaylists/%s/tracks", _base_url, playlist_id);
  char data[_max_char_size];
  int data_size = 0;
  JsonDocument doc;
  JsonArray json_array = doc["tracks"].to<JsonArray>();
  for (int i = 0; i<size; ++i) {
    JsonDocument uri;
    uri["uri"] = uris[i];
    json_array.add(uri);
  }
  serializeJson(doc, data);
  data_size = strlen(data);
  serializeJson(doc, data, data_size);

  char payload[_max_char_size];
  int payload_size = 0;
  serializeJson(doc, payload);
  payload_size = strlen(payload);
  return RestApiDelete(url, payload_size, payload);
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
  char key[] = "ids";
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array_key(size, track_ids, payload, key);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_user_saved_tracks(int size, const char ** track_ids) {
  char url[40];
  snprintf(url, sizeof(url), "%sme/tracks", _base_url);
  char payload[_max_char_size];
  char key[] = "ids";
  int payload_size = 0;
  array_to_json_array_key(size, track_ids, payload, key);
  payload_size = strlen(payload);

  return RestApiDelete(url, payload_size, payload);
}
response Spotify::check_user_saved_tracks(int size, const char ** track_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/tracks/contains?ids=%s", _base_url, array_to_char(size, track_ids, arr));

  return RestApiGet(url, filter);
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
  char key[] = "ids";
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array_key(size, artist_user_ids, payload, key);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_artists_or_users(const char* type, int size, const char** artist_user_ids) {
  char url[100];
  snprintf(url, sizeof(url), "%sme/following?type=%s", _base_url, type);
  char key[] = "ids";
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array_key(size, artist_user_ids, payload, key);
  payload_size = strlen(payload);

  return RestApiDelete(url, payload_size, payload);
}
response Spotify::check_if_user_follows_artists_or_users(const char* type, int size, const char** artist_user_ids, JsonDocument filter) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "%sme/following/contains?type=%s&ids=%s", _base_url, type, array_to_char(size, artist_user_ids, arr));

  return RestApiGet(url, filter);
}
response Spotify::check_if_current_user_follows_playlist(const char* playlist_id, JsonDocument filter) {
  char url[_max_char_size];
  snprintf(url, sizeof(url), "%splaylists/%s/followers/contains", _base_url, playlist_id);

  return RestApiGet(url, filter);
}
#endif
//Simplified functions, formatting functions
#ifndef DISABLE_SIMPLIFIED
String Spotify::get_current_album_image_url(int image_size_idx){
  String album_url = "Something went wrong";
  JsonDocument filter;
  filter["item"]["album"]["images"] = true;
  response data = currently_playing(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    JsonArray images = data.reply["item"]["album"]["images"].as<JsonArray>();
    if (!images.isNull() && image_size_idx >= 0 && image_size_idx < images.size()) {
      album_url = images[image_size_idx]["url"].as<String>();
    } 
  }
  return album_url;
}
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
  response data = currently_playing(filter);
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
  tokens.refresh_token = strdup(_refresh_token);
  tokens.client_secret = strdup(_client_secret);
  return tokens;
}
void print_response(response response_obj){
  Serial.printf("Status: %d\n", response_obj.status_code);
  Serial.print("Reply: "); 
  serializeJsonPretty(response_obj.reply, Serial);
  Serial.println();
}