#include "SpotifyEsp32.h"
#define STACK_PRINT(tag) Serial.printf("[%s] free stack: %d\n", tag, uxTaskGetStackHighWaterMark(NULL));
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

void Spotify::begin(){
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

    char encoded_redirect_uri[256];
    char encoded_scopes[768];
    urlEncode(_redirect_uri, encoded_redirect_uri, sizeof(encoded_redirect_uri));
    urlEncode(scopes, encoded_scopes, sizeof(encoded_scopes));

    char url[1024];
    snprintf(url, sizeof(url),"https://accounts.spotify.com/authorize" "?client_id=%s" "&response_type=code" "&redirect_uri=%s" "&scope=%s" "&state=%s", _client_id, encoded_redirect_uri, encoded_scopes, _random_state);

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

void Spotify::handle_client(){
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

void Spotify::set_scopes(const char* scopes){
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
response Spotify::RestApi(const char* rest_url, const char* type, int payload_size, const char* payload, JsonDocument filter, const char* content_type) {
    response response_obj;
    init_response(&response_obj);
    _client.stop();

    if (!_client.connect(_host, 443)) {
        SPOTIFY_LOGE(_TAG, "Connection failed");
        deserializeJson(response_obj.reply, "Connection failed");
        return response_obj;
    }

    SPOTIFY_LOGV(_TAG, "Creating %s request to %s", type, rest_url);

    _client.print(type); _client.print(" "); _client.print(rest_url); _client.println(" HTTP/1.1");
    _client.print("Host: "); _client.println(_host);
    _client.println("User-Agent: ESP32");
    _client.println("Connection: close");
    _client.println("Accept: application/json");
    _client.print("Authorization: Bearer "); _client.println(_access_token);
    _client.print("Content-Type: "); _client.println(content_type);

    if (strcmp(type, "GET") == 0) {
        _client.println();
    } else if (strcmp(type, "PUT") == 0 || strcmp(type, "POST") == 0 || strcmp(type, "DELETE") == 0) {
        SPOTIFY_LOGV(_TAG, "Sending payload %s with size %i ", payload ? payload : "NULL", payload_size);
        _client.print("Content-Length: "); _client.println(payload_size);  // ← fixed
        _client.println();
        if (payload_size > 0 && payload != nullptr) {
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
    JsonDocument response_doc;

    if (!valid_http_code(header_data.http_code)) response_doc = process_response(header_data);
    else response_doc = process_response(header_data, filter);

    serializeJson(response_doc, str_response);

    SPOTIFY_LOGD(_TAG, "%s \"%s\" Status: %i", type, extract_endpoint(rest_url).c_str(), header_data.http_code);
    SPOTIFY_LOGD(_TAG, "Reply: %s", str_response.c_str());

    if (_retry <= _max_num_retry && !valid_http_code(header_data.http_code)) {
        SPOTIFY_LOGD(_TAG, "Retrying");
        String message = response_doc["error"]["message"].as<String>();
        _retry++;
        if (message == "Only valid bearer authentication supported") {
            _client.stop();
            if (get_token()) {
                return RestApi(rest_url, type, payload_size, payload, filter, content_type);
            } else {
                SPOTIFY_LOGE(_TAG, "Failed to get new access token");
                deserializeJson(response_obj.reply, "Failed to get token");
                response_obj.status_code = -1;
            }
        } else {
            response_obj.reply = response_doc;
        }
    } else {
        response_obj.reply = response_doc;
    }

    _client.stop();
    _retry = 0;
    return response_obj;
}

response Spotify::RestApiPut(const char* rest_url, int payload_size, const char* payload, const char* content_type){
  return RestApi(rest_url, "PUT", payload_size, payload, JsonDocument(), content_type);
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

bool Spotify::get_refresh_token(const char* auth_code, const char* redirect_uri){
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

header_resp Spotify::process_headers(){
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

JsonDocument Spotify::process_response(header_resp header_data, JsonDocument filter){
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

#ifndef DISABLE_LIBRARY
response Spotify::save_items_to_library(int size, const char** uris){
  char url[_standard_url_size + _max_char_size];
  char uris_param[_max_char_size];
  int pos = snprintf(uris_param, sizeof(uris_param), "uris=");
  char temp[256];
  for(int i = 0; i < size; i++){
    if(i > 0) pos += snprintf(uris_param + pos, sizeof(uris_param) - pos, ",");
    urlEncode(uris[i], temp, sizeof(temp));
    pos += snprintf(uris_param + pos, sizeof(uris_param) - pos, "%s", temp);
  }
  snprintf(url, sizeof(url), "%sme/library?%s", _base_url, uris_param);
  return RestApiPut(url);
}

response Spotify::remove_items_from_library(int size, const char** uris){
  char url[_standard_url_size + _max_char_size];
  char uris_param[_max_char_size];
  int pos = snprintf(uris_param, sizeof(uris_param), "uris=");
  char temp[256];
  for (int i = 0; i < size; i++) {
    if (i > 0) pos += snprintf(uris_param + pos, sizeof(uris_param) - pos, ",");
    urlEncode(uris[i], temp, sizeof(temp));
    pos += snprintf(uris_param + pos, sizeof(uris_param) - pos, "%s", temp);
  }
  snprintf(url, sizeof(url), "%sme/library?%s", _base_url, uris_param);
  return RestApiDelete(url);
}

response Spotify::check_users_saved_items(int size, const char** uris, JsonDocument filter){
  char url[_standard_url_size + _max_char_size];
  char uris_param[_max_char_size];
  int pos = snprintf(uris_param, sizeof(uris_param), "uris=");
  char temp[256];
  for (int i = 0; i < size; i++) {
    if (i > 0) pos += snprintf(uris_param + pos, sizeof(uris_param) - pos, ",");
    urlEncode(uris[i], temp, sizeof(temp));
    pos += snprintf(uris_param + pos, sizeof(uris_param) - pos, "%s", temp);
  }
  snprintf(url, sizeof(url), "%sme/library/contains?%s", _base_url, uris_param);
  return RestApiGet(url, filter);
}

response Spotify::get_users_saved_albums(int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/albums?limit=%d&offset=%d", _base_url, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_users_saved_audiobooks(int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/audiobooks?limit=%d&offset=%d", _base_url, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_users_saved_episodes(int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/episodes?limit=%d&offset=%d", _base_url, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_users_saved_shows(int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/shows?limit=%d&offset=%d", _base_url, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_users_saved_tracks(int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/tracks?limit=%d&offset=%d", _base_url, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_followed_artists(int limit, const char* after, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/following?type=artist&limit=%d", _base_url, limit);
  if (after != nullptr) {
    char encoded[256];
    urlEncode(after, encoded, sizeof(encoded));
    char tmp[300];
    snprintf(tmp, sizeof(tmp), "&after=%s", encoded);
    strncat(url, tmp, sizeof(url) - strlen(url) - 1);
  }
  return RestApiGet(url, filter);
}
#endif

#ifndef DISABLE_PLAYLISTS
response Spotify::change_playlist_details(const char* playlist_id, JsonDocument details){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s", _base_url, playlist_id);
  char payload[512];
  serializeJson(details, payload, sizeof(payload));
  int payload_size = strlen(payload);
  return RestApiPut(url, payload_size, payload);
}

response Spotify::create_playlist(const char* name, const char* description, bool is_public, bool collaborative){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/playlists", _base_url);

  JsonDocument doc;
  doc["name"] = name;
  if (description != nullptr) doc["description"] = description;
  doc["public"] = is_public;
  doc["collaborative"] = collaborative;

  char payload[256];
  serializeJson(doc, payload, sizeof(payload));
  int payload_size = strlen(payload);
  return RestApiPost(url, payload_size, payload);
}

response Spotify::get_current_users_playlists(int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/playlists?limit=%d&offset=%d", _base_url, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_playlist(const char* playlist_id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s", _base_url, playlist_id);
  return RestApiGet(url, filter);
}

response Spotify::get_playlist_cover_image(const char* playlist_id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s/images", _base_url, playlist_id);
  return RestApiGet(url, filter);
}

response Spotify::upload_custom_playlist_cover_image(const char* playlist_id, const char* image_data){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s/images", _base_url, playlist_id);
  int payload_size = strlen(image_data);
  return RestApiPut(url, payload_size, image_data, "image/jpeg");
}

response Spotify::add_items_to_playlist(const char* playlist_id, int size, const char** uris, int position){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s/items", _base_url, playlist_id);
  if (position >= 0) {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "?position=%d", position);
    strncat(url, tmp, sizeof(url) - strlen(url) - 1);
  }

  JsonDocument doc;
  JsonArray arr = doc["uris"].to<JsonArray>();
  for (int i = 0; i < size; i++) arr.add(uris[i]);

  char payload[_max_char_size];
  serializeJson(doc, payload, sizeof(payload));
  int payload_size = strlen(payload);
  return RestApiPost(url, payload_size, payload);
}

response Spotify::remove_playlist_items(const char* playlist_id, int size, const char** uris, const char* snapshot_id){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s/items", _base_url, playlist_id);

  JsonDocument doc;
  JsonArray arr = doc["items"].to<JsonArray>();
  for (int i = 0; i < size; i++) {
    if (uris[i]) {
      JsonObject obj = arr.add<JsonObject>();
      obj["uri"] = uris[i];
    }
  }
  if (snapshot_id != nullptr) doc["snapshot_id"] = snapshot_id;

  char payload[_max_char_size];
  serializeJson(doc, payload, sizeof(payload));
  int payload_size = strlen(payload);
  return RestApiDelete(url, payload_size, payload);
}

response Spotify::update_playlist_items(const char* playlist_id, int size, const char** uris, int range_start, int insert_before, int range_length, const char* snapshot_id){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s/items", _base_url, playlist_id);

  JsonDocument doc;
  if (size > 0) {
    JsonArray arr = doc["uris"].to<JsonArray>();
    for (int i = 0; i < size; i++) arr.add(uris[i]);
  } else {
      doc["range_start"] = range_start;
    doc["insert_before"] = insert_before;
    doc["range_length"] = range_length;
  }
  if (snapshot_id != nullptr) doc["snapshot_id"] = snapshot_id;

  char payload[_max_char_size];
  serializeJson(doc, payload, sizeof(payload));
  int payload_size = strlen(payload);
  return RestApiPut(url, payload_size, payload);
}

response Spotify::get_playlist_items(const char* playlist_id, int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%splaylists/%s/items?limit=%d&offset=%d", _base_url, playlist_id, limit, offset);
  return RestApiGet(url, filter);
}
#endif

#ifndef DISABLE_METADATA
response Spotify::get_album(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%salbums/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::get_album_tracks(const char* id, int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%salbums/%s/tracks?limit=%d&offset=%d", _base_url, id, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_artist(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sartists/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::get_artists_albums(const char* id, int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sartists/%s/albums?limit=%d&offset=%d", _base_url, id, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_audiobook(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%saudiobooks/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::get_audiobook_chapters(const char* id, int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%saudiobooks/%s/chapters?limit=%d&offset=%d", _base_url, id, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_chapter(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%schapters/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::get_episode(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sepisodes/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::get_show(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sshows/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::get_show_episodes(const char* id, int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sshows/%s/episodes?limit=%d&offset=%d", _base_url, id, limit, offset);
  return RestApiGet(url, filter);
}

response Spotify::get_track(const char* id, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%stracks/%s", _base_url, id);
  return RestApiGet(url, filter);
}

response Spotify::search_for_item(const char* q, const char* type, int limit, int offset, JsonDocument filter){
  char url[_standard_url_size];
  char encoded_q[512];
  urlEncode(q, encoded_q, sizeof(encoded_q));
  snprintf(url, sizeof(url), "%ssearch?q=%s&type=%s&limit=%d&offset=%d", _base_url, encoded_q, type, limit, offset);
  return RestApiGet(url, filter);
}
#endif

#ifndef DISABLE_USER
response Spotify::get_current_users_profile(JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme", _base_url);
  return RestApiGet(url, filter);
}

response Spotify::get_users_top_items(const char* type, int limit, int offset, const char* time_range, JsonDocument filter){
  char url[_standard_url_size * 2];
  int len = snprintf(url, sizeof(url),"%sme/top/%s?limit=%d&offset=%d",_base_url, type, limit, offset);
  if (time_range != nullptr && len < sizeof(url)) {
    snprintf(url + len, sizeof(url) - len, "&time_range=%s", time_range);
  }
  return RestApiGet(url, filter);
}
#endif

#ifndef DISABLE_PLAYER
response Spotify::add_item_to_queue(const char* uri, const char* device_id){
  char url[_max_char_size];
  char encoded_uri[256];
  urlEncode(uri, encoded_uri, sizeof(encoded_uri));

  if (device_id == nullptr) {
    snprintf(url, sizeof(url), "%sme/player/queue?uri=%s", _base_url, encoded_uri);
  } else {
    snprintf(url, sizeof(url), "%sme/player/queue?uri=%s&device_id=%s", _base_url, encoded_uri, device_id);
  }
  return RestApiPost(url);
}

response Spotify::get_available_devices(JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/devices", _base_url);
  return RestApiGet(url, filter);
}

response Spotify::get_currently_playing_track(JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/currently-playing", _base_url);
  return RestApiGet(url, filter);
}

response Spotify::get_playback_state(JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player", _base_url);
  return RestApiGet(url, filter);
}

response Spotify::get_recently_played_tracks(int limit, JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/recently-played?limit=%d", _base_url, limit);
  return RestApiGet(url, filter);
}

response Spotify::get_users_queue(JsonDocument filter){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/queue", _base_url);
  return RestApiGet(url, filter);
}

response Spotify::pause_playback(){
  char url[_standard_url_size];
  snprintf(url, sizeof(url),"%sme/player/pause", _base_url);
  return RestApiPut(url);
}

response Spotify::seek_to_position(int position_ms){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/seek?position_ms=%d", _base_url, position_ms);
  return RestApiPut(url);
}

response Spotify::set_repeat_mode(const char* state){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/repeat?state=%s", _base_url, state);
  return RestApiPut(url);
}

response Spotify::set_volume(int volume_percent){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/volume?volume_percent=%d", _base_url, volume_percent);
  return RestApiPut(url);
}

response Spotify::skip_to_next(){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/next", _base_url);
  return RestApiPost(url);
}

response Spotify::skip_to_previous(){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/previous", _base_url);
  return RestApiPost(url);
}

response Spotify::start_a_users_playback(const char* device_id){
  char url[_standard_url_size];
  if (device_id == nullptr)
      snprintf(url, sizeof(url), "%sme/player/play", _base_url);
  else
      snprintf(url, sizeof(url), "%sme/player/play?device_id=%s", _base_url, device_id);
  return RestApiPut(url);
}

response Spotify::toggle_shuffle(bool state){
  char url[_standard_url_size];
  snprintf(url, sizeof(url), "%sme/player/shuffle?state=%s", _base_url, state ? "true" : "false");
  return RestApiPut(url);
}

response Spotify::transfer_playback(const char* device_id){
  char url[_standard_url_size];
  snprintf(url, sizeof(url),"%sme/player", _base_url);
  char payload[100];
  int payload_size = 0;
  snprintf(payload,sizeof(payload), "{\"device_ids\":[\"%s\"]}", device_id);
  payload_size = strlen(payload);
  return RestApiPut(url, payload_size, payload);
}
#endif

#ifndef DISABLE_SIMPLIFIED
String Spotify::get_current_album_image_url(int image_size_idx){
  String album_url = "Something went wrong";
  JsonDocument filter;
  filter["item"]["album"]["images"] = true;
  response data = get_currently_playing_track(filter);
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
  response data = get_currently_playing_track(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    track_name = data.reply["item"]["name"].as<String>();
  }
  return track_name;
}

String Spotify::current_track_id(){
  String track_id = "Something went wrong";
  JsonDocument filter;
  filter["item"]["id"] = true;
  response data = get_currently_playing_track(filter);
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
  response data = get_available_devices(filter);
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
  response data = get_currently_playing_track(filter);
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
  response data = get_currently_playing_track(filter);
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
  response data = get_currently_playing_track(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    strcpy(track_name, data.reply["item"]["name"].as<String>().c_str());
  }
  return track_name;
}

char* Spotify::current_track_id(char * track_id){
  JsonDocument filter;
  filter["item"]["id"] = true;
  response data = get_currently_playing_track(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    strcpy(track_id, data.reply["item"]["id"].as<String>().c_str());
  }
  return track_id;
}

char* Spotify::current_artist_names(char * artist_names){
  JsonDocument filter;
  filter["item"]["artists"] = true;
  response data = get_currently_playing_track(filter);
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
  response data = get_currently_playing_track(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    is_playing = data.reply["is_playing"].as<bool>();
  }
  return is_playing;
}

bool Spotify::volume_modifyable(){
  bool volume_modifyable = false;
  JsonDocument filter;
  filter["device"]["supports_volume"] = true;
  response data = get_playback_state(filter);
  if(valid_http_code(data.status_code) && !data.reply.isNull()){
    volume_modifyable = data.reply["device"]["supports_volume"];
  }
  return volume_modifyable;
}
#endif

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

void Spotify::urlEncode(const char* src, char* dest, size_t destSize){
  size_t j = 0;
  for(size_t i = 0; src[i] != '\0' && j < destSize - 1; ++i){
    unsigned char c = src[i];
    if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_' || c == '.' || c == '~'){
      dest[j++] = c;
    }else{
      if(j + 3 < destSize){
        snprintf(dest + j, 4, "%%%02X", c);
        j += 3;
      }else{
        break;
      }
    }
  }
  dest[j] = '\0';
}

void Spotify::async(std::function<response()> fn, AsyncCallback callback) {
  struct TaskParam {
    std::function<response()> fn;
    AsyncCallback callback;
  };
  TaskParam* param = new TaskParam{fn, callback};

  xTaskCreatePinnedToCore(
      [](void* pv) {
          TaskParam* p = (TaskParam*)pv;
          response r = p->fn();
          if (p->callback) p->callback(r);
          delete p;
          vTaskDelete(NULL);
      },
      "SpotifyAsync", 8192, param, 1, NULL, 1);
}