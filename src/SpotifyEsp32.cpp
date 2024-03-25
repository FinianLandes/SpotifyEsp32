#include "SpotifyEsp32.h"
namespace Spotify_types {
  bool SHUFFLE_ON = true;
  bool SHUFFLE_OFF = false;
  char* REPEAT_OFF = "off";
  char* REPEAT_TRACK = "track";
  char* REPEAT_CONTEXT = "context";
  char* TYPE_ALBUM = "album";
  char* TYPE_ARTIST = "artist";
  char* TYPE_TRACK = "track";
  char* TYPE_PLAYLIST = "playlist";
  char* TYPE_SHOW = "show";
  char* TYPE_EPISODE = "episode";
  char* TYPE_AUDIOBOOK = "audiobook";
  char* TOP_TYPE_ARTIST = "artists";
  char* TOP_TYPE_TRACKS = "tracks";
  char* GROUP_ALBUM = "album";
  char* GROUP_SINGLE = "single";
  char* GROUP_APPEARS_ON = "appears_on";
  char* GROUP_COMPILATION = "compilation";
  char* TIME_RANGE_SHORT = "short_term";
  char* TIME_RANGE_MEDIUM = "medium_term";
  char* TIME_RANGE_LONG = "long_term";
  char* FOLLOW_TYPE_ARTIST = "artist";
  char* FOLLOW_TYPE_USER = "user";
  int SIZE_OF_ID = 40;
  int SIZE_OF_URI = 50;
  int SIZE_OF_SECRET_ID = 100;
  int SIZE_OF_REFRESH_TOKEN = 300;
}

Spotify::Spotify(const char* client_id, const char* client_secret, int server_port, bool debug_on, int max_num_retry):_server(server_port){
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
}

Spotify::Spotify(const char* client_id, const char* client_secret, const char* refresh_token, int server_port,bool debug_on, int max_num_retry):_server(server_port){
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
}
Spotify::~Spotify(){
  if(_debug_on){
    Serial.println("Object Destructed");
  }
}
//Login Webserver Functions

std::function<void()> callback_fn_refresh(Spotify *spotify) {
  return [spotify]() {
        return spotify->server_on_refresh();
    };
};
std::function<void()> callback_fn_root(Spotify *spotify) {
  return [spotify]() {
        return spotify->server_on_root();
    };
};
std::function<void()> callback_fn_response(Spotify *spotify) {
  return [spotify]() {
        return spotify->server_on_response();
    };
};
bool Spotify::get_refresh_token() {
  bool reply = false;
  if (!_client.connect("accounts.spotify.com", 443)) {
    return false;
  }

  String authorization = String(_client_id) + ":" + String(_client_secret);
  authorization.trim();
  authorization = "Basic " + base64::encode(authorization);

  String payload = "grant_type=authorization_code&code=" + String(_auth_code) + "&redirect_uri=" + String(_redirect_uri)+ "callback";

  _client.println("POST /api/token HTTP/1.1");
  _client.println("Host: accounts.spotify.com");
  _client.println("User-Agent: ESP32");
  _client.println("Connection: close");
  _client.println("Content-Type: application/x-www-form-urlencoded");
  _client.println("Authorization: " + authorization);
  _client.println("Content-Length: " + String(payload.length()));
  _client.println();
  _client.println(payload);

  String line;
  int http_code = -1;
  while (_client.connected()) {
    line = _client.readStringUntil('\n');
    if (line.startsWith("HTTP/1.1")) {
      http_code = line.substring(9, 12).toInt();
    }
    if (line == "\r") {
      break;
    }
  }

  String response;
  while (_client.available()) {
    response += _client.readStringUntil('\n');
  }

  if (_debug_on) {
    Serial.printf("POST \"Refresh token\" Status: %d \n", http_code);
    Serial.printf("Reply: %s\n", response.c_str());
  }

  if (http_code >= 200 && http_code <= 299) {
    JsonDocument doc;
    deserializeJson(doc, response);
    const char* temp = doc["refresh_token"];
    strncpy(_refresh_token, temp, sizeof(_refresh_token));
    Serial.printf("Refresh Token: %s\n", _refresh_token);
    reply = true;
  } else {
    reply = false;
  }

  _client.stop();
  return reply;
}
void Spotify::server_on_root() {
  if((strcmp(_client_id, "") == 0 || !_client_id || strcmp(_client_secret, "") == 0 || !_client_secret)){
    _server.send(200,"text/html", String(_credentials_input));
  }else{
    _server.sendHeader("Location", String(_redirect_uri)+"callback");
    _server.send(302, "text/plain", "");
  }
}
void Spotify::server_on_response(){
  if(_server.args() != 0){
    if(_server.hasArg("id")){
      strncpy(_client_id, _server.arg("id").c_str(), sizeof(_client_id));
    }
    if(_server.hasArg("secret")){
      strncpy(_client_secret, _server.arg("secret").c_str(), sizeof(_client_secret));
    }
    if(_server.hasArg("token")){
      strncpy(_refresh_token, _server.arg("token").c_str(), sizeof(_refresh_token));
    }
    if (strcmp(_refresh_token, "") != 0) {
      _server.send(200, "text/html", "Credentials successfully set!");
      _server.stop();
    } else if (strcmp(_refresh_token, "") == 0) {
      _server.sendHeader("Location", String(_redirect_uri)+"callback");
      _server.send(302, "text/plain", "");
    }
  }else{
    _server.send(200, "test/html", "Something went wrong, please try again");
  }
}
void Spotify::server_on_refresh() {
  if (strcmp(_refresh_token, "") == 0 || !_refresh_token) {
    if (_server.args() == 0) {
      char page[900];
      char redirect_callback[150];
      snprintf(redirect_callback, sizeof(redirect_callback),"%scallback", _redirect_uri);
      snprintf(page,sizeof(page),_login_page, _client_id, redirect_callback);
      _server.send(200, "text/html", String(page));
    } else {
      if (_server.hasArg("code")) {
        strncpy(_auth_code, _server.arg("code").c_str(), sizeof(_auth_code));
        if(_debug_on){
          Serial.printf("Auth Code: %s\n", _auth_code);
        }
        if(get_refresh_token()){
          char message[500];
          snprintf(message,sizeof(message), "Setup Complete, Refresh Token: %s <br>You can now close this page", _refresh_token);
          _server.send(200, "text/html", message);
        }
        else{
          _server.send(200, "text/html", "Something went wrong, please try again");
        }
        _server.stop();
      } else {
        char page[900];
        snprintf(page,sizeof(page),_login_page, _client_id, _redirect_uri);
        _server.send(200, "text/html", String(page));
      }
    }
  } else {
    _server.send(200, "text/html", "Spotify setup complete");
    _server.stop();
  }
}
void Spotify::server_routes(){
  _server.on("/", HTTP_GET, [this](){
    if(_debug_on){
      Serial.println("Send response to root");
    }
    auto root = callback_fn_root(this);
    root();
  });
  _server.on("/callback", HTTP_GET, [this](){
    if(_debug_on){
      Serial.println("Send response to Callback");
    }
    auto refresh = callback_fn_refresh(this);
    refresh();
  });
  
  _server.on("/get", HTTP_GET, [this](){
    if(_debug_on){
      Serial.println("Receive credentials from root");
    }
    auto response = callback_fn_response(this);
    response();
  });
  _server.begin();
  if(_debug_on){
    Serial.println("Server started");
  }
}
void Spotify::begin(){ 
  if(_port == 80){
    snprintf(_redirect_uri,sizeof(_redirect_uri), "http://%s/", WiFi.localIP().toString().c_str());
    Serial.printf("Go to this url in your Browser to login to spotify or enter your credentials: %s\n", _redirect_uri);
  }
  else{
    snprintf(_redirect_uri,sizeof(_redirect_uri), "http://%s:%d/", WiFi.localIP().toString().c_str(), _port);
    Serial.printf("Go to this url in your Browser to login to spotify or enter your credentials: %s\n", _redirect_uri);
  }
  _client.setCACert(_spotify_root_ca);
  if(!is_auth()){
    server_routes();
  }
}

void Spotify::end(){
  delete this;
}
void Spotify::handle_client(){
  _server.handleClient(); 
}

bool Spotify::is_auth(){
  return !(strcmp(_refresh_token, "") == 0 || !_refresh_token || strcmp(_client_id, "") == 0 || !_client_id || strcmp(_client_secret, "") == 0 || !_client_secret) ;
}

//Basic functions
response Spotify::RestApi(char* rest_url, char* type, int payload_size, char* payload){
  response response_obj;
  int http_code = -1;
  String reply = "";
  JsonDocument doc;
  init_response(&response_obj);

  if (!_client.connect(_host, 443)) {
    response_obj.reply = "Connection failed";
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
  int line_num = 0;
  while (_client.connected()) {
    String line = _client.readStringUntil('\n'); 
    if(!line_num){
      http_code = line.substring(9,12).toInt();
    }
    if (line == "\r") {
      break;
    }
    line_num++;
    if(_debug_on){
      Serial.println(line);
    }
  }
  response_obj.status_code = http_code;
  while(_client.available()){
    reply = reply + _client.readStringUntil('\n') + "\n";
  }
  deserializeJson(doc, reply);
  if(_debug_on){
    const char* endpoint = extract_endpoint(rest_url);
    Serial.printf("%s \"%s\" Status: %i\n", type, endpoint, http_code);
    Serial.printf("Reply: %s\n", reply.c_str());
  }
  if ((http_code >= 200)&&(http_code <= 299)){
    response_obj.reply = reply;    
  }
  else if(_retry<=_max_num_retry){
    String message = doc["error"]["message"].as<String>();
    if(message == "Only valid bearer authentication supported"){
      _retry++;
      if(get_token()){
        response_obj = RestApi(rest_url, type, payload_size, payload);
      }
    }
    else{
      response_obj.reply = message;
    }
  }
  
  _client.stop();
  return response_obj;
}

response Spotify::RestApiPut(char* rest_url,int payload_size, char* payload){
  return RestApi(rest_url, "PUT", payload_size, payload);
}
response Spotify::RestApiGet(char* rest_url){
  return RestApi(rest_url, "GET");
}
response Spotify::RestApiPost(char* rest_url,int payload_size, char* payload){
  return RestApi(rest_url, "POST", payload_size, payload);
}
response Spotify::RestApiDelete(char* rest_url, char* payload){
  return RestApi(rest_url, "DELETE", 0, payload);
}
bool Spotify::get_token() {
  bool reply = false;
  if (!_client.connect("accounts.spotify.com", 443)) {
    return false;
  }
  String authorization = String(_client_id) + ":" + String(_client_secret);
  authorization.trim();
  authorization = "Basic " + base64::encode(authorization);

  String payload = "grant_type=refresh_token&refresh_token=" + String(_refresh_token);

  _client.println("POST /api/token HTTP/1.1");
  _client.println("Host: accounts.spotify.com");
  _client.println("User-Agent: ESP32");
  _client.println("Connection: close");
  _client.println("Content-Type: application/x-www-form-urlencoded");
  _client.println("Authorization: " + authorization);
  _client.println("Content-Length: " + String(payload.length()));
  _client.println();
  _client.println(payload);

  String line;
  int http_code = -1;
  while (_client.connected()) {
    line = _client.readStringUntil('\n');
    if (line.startsWith("HTTP/1.1")) {
      http_code = line.substring(9, 12).toInt();
    }
    if (line == "\r") {
      break;
    }
  }

  String response;
  while (_client.available()) {
    response += _client.readStringUntil('\n');
  }

  if (_debug_on) {
    Serial.printf("POST \"token\" Status: %d \n", http_code);
    Serial.printf("Reply: %s\n", response.c_str());
  }

  if (http_code >= 200 && http_code <= 299) {
    JsonDocument doc;
    deserializeJson(doc, response);
    _access_token = doc["access_token"].as<String>();
    reply = true;
  } else {
    reply = false;
  }

  _client.stop();
  return reply;
}

void Spotify::init_response(response* response_obj){
  response_obj -> status_code = -1;
  response_obj -> reply ="If you see this something went wrong, if the issue persists please contact the developer";
}
char* Spotify::array_to_char(int size, char** array, char* result) {
  result[0] = '\0';
  for (int i = 0; i < size; ++i) {
    strcat(result, array[i]);
    if (i != size - 1) {
      strcat(result, ",");
    }
  }
  return result;
}
void Spotify::array_to_json_array(int size, char** array, char* data, int data_size){
  JsonDocument doc;
  JsonArray json_array = doc.to<JsonArray>();
  for (int i = 0; i<size; ++i) {
    json_array.add(array[i]);
  }
  serializeJson(json_array,data,data_size);
}
const char* Spotify::extract_endpoint(const char* url){
  std::regex pattern(R"(https://api\.spotify\.com/v1/([^?]+))");
    std::cmatch match;
    if (std::regex_search(url, match, pattern)) {
      return match[1].str().c_str();
    }
    return nullptr;
}
//API functions
//Player
#ifdef ENABLE_PLAYER
response Spotify::currently_playing(){
  char url[] = "https://api.spotify.com/v1/me/player/currently-playing";
  return RestApiGet(url);
}
response Spotify::current_playback_state(){
  char url[] = "https://api.spotify.com/v1/me/player";
  return RestApiGet(url);
}
response Spotify::start_resume_playback(char* context_uri, int offset, int position_ms, char* device_id){
  char url[100] = "https://api.spotify.com/v1/me/player/play";
  if(device_id != nullptr){
    snprintf(url,sizeof(url), "https://api.spotify.com/v1/me/player/play?device_id=%s", device_id);
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
response Spotify::start_resume_playback(int size, char** uris, char* device_id){
  char url[100] = "https://api.spotify.com/v1/me/player/play";
  if(device_id != nullptr){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/play?device_id=%s", device_id);
  }
  char payload[_max_char_size]; 
  int payload_size = 0;
  char arr[_max_char_size];
  array_to_json_array(size, uris, arr);
  snprintf(payload,sizeof(payload), "{\"uris\":%s}", arr);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::start_resume_playback(char* device_id){
  char url[100];
  if(device_id != nullptr){
    snprintf(url, sizeof(url),"https://api.spotify.com/v1/me/player/play?device_id=%s", device_id);
  }
  else{
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/play");
  }
  
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
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/recently-played?limit=%d", limit);
  return RestApiGet(url);
}
response Spotify::get_queue(){
  char url[] = "https://api.spotify.com/v1/me/player/queue";
  return RestApiGet(url);
}
response Spotify::transfer_playback(char* device_id){
  char url[] = "https://api.spotify.com/v1/me/player";
  char payload[100];
  int payload_size = 0;
  snprintf(payload,sizeof(payload), "{\"device_ids\":[\"%s\"]}", device_id);
  payload_size = strlen(payload);
  return RestApiPut(url,payload_size, payload);
}
response Spotify::seek_to_position(int time_ms){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/seek?position_ms=%d", time_ms);

  return RestApiPut(url);
}
response Spotify::repeat_mode(char* mode){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/repeat?state=%s", mode);

  return RestApiPut(url);
}
response Spotify::set_volume(int value){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/volume?volume_percent=%d", value);

  return RestApiPut(url);
}
response Spotify::shuffle(bool mode){
  char state[6]; 
  snprintf(state,sizeof(state), "%s", mode ? "true" : "false");

  char url[100]; 
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/shuffle?state=%s", state);

  return RestApiPut(url);
}
response Spotify::add_to_queue(char* context_uri){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/queue?uri=%s", context_uri);

  return RestApiPost(url);
}
#endif
#ifdef ENABLE_ALBUM
//Albums
response Spotify::get_album(char* album_id){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/albums/%s", album_id);
  
  return RestApiGet(url);
}
response Spotify::get_albums(int size, char ** album_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/albums?ids=%s", array_to_char(size, album_ids,arr));
  
  return RestApiGet(url);
}
response Spotify::get_album_tracks(char* album_id, int limit, int offset){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/albums/%s/tracks?limit=%d&offset=%d", album_id, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_users_saved_albums(int limit, int offset){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/albums?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_albums_for_current_user(int size, char ** album_ids){
  char url[] = "https://api.spotify.com/v1/me/albums";
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, album_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_users_saved_albums(int size,char ** album_ids){
  char url[] = "https://api.spotify.com/v1/me/albums";
  char payload[_max_char_size];
  array_to_json_array(size, album_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_useres_saved_albums(int size,char ** album_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/albums/contains?ids=%s", array_to_char(size, album_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_new_releases(int limit, int offset, char* country){
  char url[120];
  if(country == nullptr){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/new-releases?limit=%d&offset=%d", limit, offset);
  }
  else{
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/new-releases?country=%s&limit=%d&offset=%d", country, limit, offset);
  }

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_ARTIST
//Artists
response Spotify::get_artist(char* artist_id){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/artists/%s", artist_id);

  return RestApiGet(url);
}
response Spotify::get_several_artists(int size, char ** artist_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/artists?ids=%s", array_to_char(size, artist_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_artist_albums(char* artist_id,int size_groups, char** include_groups, int limit, int offset){
  char url[200];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/artists/%s/albums?include_groups=%s&limit=%d&offset=%d", artist_id, array_to_char(size_groups,include_groups, arr), limit, offset);  

  return RestApiGet(url);
}
response Spotify::get_artist_top_tracks(char* artist_id, char* country){
  char url[100];
  if(country == nullptr){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/artists/%s/top-tracks", artist_id);
  }
  else{
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/artists/%s/top-tracks?country=%s", artist_id, country);
  }

  return RestApiGet(url);
}
response Spotify::get_artist_related_artist(char* artist_id){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/artists/%s/related-artists", artist_id);

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_AUDIOBOOKS
//Audiobooks
response Spotify::get_audiobook(char* audiobook_id){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/audiobooks/%s", audiobook_id);

  return RestApiGet(url);
}
response Spotify::get_several_audiobooks(int size, char** audiobook_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/audiobooks?ids=%s", array_to_char(size, audiobook_ids,arr));

  return RestApiGet(url);
}
response Spotify::get_audiobook_chapters(char* audiobook_id, int limit, int offset){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/audiobooks/%s/chapters?limit=%d&offset=%d", audiobook_id, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_users_audiobooks(int limit, int offset){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/audiobooks?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_audiobooks_for_current_user(int size, char** audiobook_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/audiobooks?ids=%s", array_to_char(size, audiobook_ids, arr));

  return RestApiPut(url);
}
response Spotify::remove_audiobooks_for_current_user(int size, char** audiobook_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/audiobooks?ids=%s", array_to_char(size, audiobook_ids, arr));

  return RestApiDelete(url);
}
response Spotify::check_users_saved_audiobooks(int size, char** audiobook_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/audiobooks/contains?ids=%s", array_to_char(size, audiobook_ids,arr));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_CATEGORIES
//Categories
response Spotify::get_several_browse_categories(int limit, int offset, char* country, char* locale){
  char url[200];
  if((country == nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories?limit=%d&offset=%d", limit, offset);
  }
  else if((country == nullptr)&&(locale != nullptr)){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories?locale=%s&limit=%d&offset=%d", locale, limit, offset);
  }
  else if((country != nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories?country=%s&limit=%d&offset=%d", country, limit, offset);
  }
  else{
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories?country=%s&locale=%s&limit=%d&offset=%d", country, locale, limit, offset);
  }

  return RestApiGet(url);
}
response Spotify::get_single_browse_category(char* category_id, char* country, char* locale){
  char url[150];
  if((country == nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories/%s", category_id);
  }
  else if((country == nullptr)&&(locale != nullptr)){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories/%s?locale=%s", category_id, locale);
  }
  else if((country != nullptr)&&(locale == nullptr)){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories/%s?country=%s", category_id, country);
  }
  else{
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories/%s?country=%s&locale=%s", category_id, country, locale);
  }

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_CHAPTERS
//Chapters
response Spotify::get_chapter(char* chapter_id){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/chapters/%s", chapter_id);

  return RestApiGet(url);
}
response Spotify::get_several_chapters(int size, char** chapter_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/chapters?ids=%s", array_to_char(size, chapter_ids, arr));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_EPISODES
//Episodes
response Spotify::get_episode(char* episode_id){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/episodes/%s", episode_id);

  return RestApiGet(url);
}
response Spotify::get_several_episodes(int size,char** episode_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/episodes?ids=%s", array_to_char(size, episode_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_users_saved_episodes(int limit, int offset){
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/episodes?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_episodes_for_current_user(int size,char** episode_ids){
  char url[] = "https://api.spotify.com/v1/me/episodes";
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, episode_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_episodes_for_current_user(int size,char** episode_ids){
  char url[] = "https://api.spotify.com/v1/me/episodes";
  char payload[_max_char_size];
  array_to_json_array(size, episode_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_users_saved_episodes(int size,char** episode_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/episodes/contains?ids=%s", array_to_char(size, episode_ids, arr));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_GENRES
//Genres
response Spotify::get_available_genre_seeds(){
  char url[] = "https://api.spotify.com/v1/recommendations/available-genre-seeds";

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_MARKETS
//Markets
response Spotify::get_available_markets(){
  char url[] = "https://api.spotify.com/v1/markets";
  
  return RestApiGet(url);
}
#endif
#ifdef ENABLE_PLAYLISTS
//Playlist
response Spotify::get_playlist(char* playlist_id,int size, char** fields,int size_of_additional_types, char** additional_types) {
  char url[200];
  if(size == 0){
    if(size_of_additional_types == 0){
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s", playlist_id);
    }
    else{
      char arr[_max_char_size];
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s?additional_types=%s", playlist_id, array_to_char(size_of_additional_types, additional_types, arr));
    }
  }
  else{
    if(size_of_additional_types == 0){
      char arr[_max_char_size];
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s?fields=%s", playlist_id, array_to_char(size, fields, arr));
    }
    else{
      char arr[_max_char_size];
      char arr2[_max_char_size];
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s?fields=%s&additional_types=%s", playlist_id, array_to_char(size, fields, arr), array_to_char(size_of_additional_types, additional_types, arr2));
    }
  }

  return RestApiGet(url);
}
response Spotify::change_playlist_details(char* playlist_id, char* name, bool is_public, bool is_collaborative, char* description) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s", playlist_id);

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
response Spotify::get_playlist_items(char* playlist_id, char* fields, int limit, int offset) {
  char url[200];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks?fields=%s&limit=%d&offset=%d", playlist_id, fields, limit, offset);

  return RestApiGet(url);
}
response Spotify::update_playlist_items(char* playlist_id,int size, char** uris, int range_length, int range_start, int insert_before) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);
  
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
response Spotify::add_items_to_playlist(char* playlist_id, int size, char ** uris, int position) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);
  
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
response Spotify::remove_playlist_items(char* playlist_id,int size, char** uris) { 
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);
  char arr[_max_char_size];
  char* payload = array_to_char(size, uris, arr);
  
  
  return RestApiDelete(url, payload);
}
response Spotify::get_current_users_playlists(int limit, int offset) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/playlists?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::get_user_playlists(char* user_id, int limit, int offset) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/users/%s/playlists?limit=%d&offset=%d", user_id, limit, offset);
  
  return RestApiGet(url);
}
response Spotify::create_playlist(char* user_id, char* name, bool is_public, bool is_collaborative, char* description) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/users/%s/playlists", user_id);

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
response Spotify::get_featured_playlists(int limit, int offset, char* timestamp, char* country, char* locale) {
    char url[200];
    if (timestamp) {
        if (!country && !locale) {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?timestamp=%s&limit=%d&offset=%d", timestamp, limit, offset);
        }
        else if (!country && locale) {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?locale=%s&timestamp=%s&limit=%d&offset=%d", locale, timestamp, limit, offset);
        }
        else if (country && !locale) {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?country=%s&timestamp=%s&limit=%d&offset=%d", country, timestamp, limit, offset);
        }
        else {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?country=%s&locale=%s&timestamp=%s&limit=%d&offset=%d", country, locale, timestamp, limit, offset);
        }
    }
    else {
        if (!country && !locale) {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?limit=%d&offset=%d", limit, offset);
        }
        else if (!country && locale) {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?locale=%s&limit=%d&offset=%d", locale, limit, offset);
        }
        else if (country && !locale) {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?country=%s&limit=%d&offset=%d", country, limit, offset);
        }
        else {
            snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/featured-playlists?country=%s&locale=%s&limit=%d&offset=%d", country, locale, limit, offset);
        }
    }
    return RestApiGet(url);
}
response Spotify::get_category_playlists(char* category_id, int limit, int offset, char* country) {
  char url[200];
  if(country == nullptr){
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories/%s/playlists?limit=%d&offset=%d", category_id, limit, offset);
  }
  else{
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/browse/categories/%s/playlists?country=%s&limit=%d&offset=%d", category_id, country, limit, offset);
  }

  return RestApiGet(url);
}
response Spotify::get_playlist_cover_image(char* playlist_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/images", playlist_id);

  return RestApiGet(url);
}
response Spotify::add_custom_playlist_cover_image(char* playlist_id, char* data) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/images", playlist_id);  
  int payload_size = strlen(data);

  return RestApiPut(url, payload_size, data);
}
#endif
#ifdef ENABLE_SEARCH
//Search
response Spotify::search(char* q, int type_size, char** type, int limit, int offset, char* market){
  char url[_max_char_size];
  char arr[_max_char_size];
  if(market){
    if(type_size == 0){
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/search?q=%s&limit=%d&offset=%d&market=%s", q, limit, offset, market);
    }
    else{
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/search?q=%s&type=%s&limit=%d&offset=%d&market=%s", q, array_to_char(type_size, type, arr), limit, offset, market);
    }
    
  }
  else{
    if(type_size == 0){
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/search?q=%s&limit=%d&offset=%d", q, limit, offset);
    }
    else{
      snprintf(url, sizeof(url), "https://api.spotify.com/v1/search?q=%s&type=%s&limit=%d&offset=%d", q, array_to_char(type_size, type, arr), limit, offset);
    }
  }

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_SHOWS
//Shows
response Spotify::get_show(char* show_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/shows/%s", show_id);

  return RestApiGet(url);
}
response Spotify::get_several_shows(int size,char ** show_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/shows?ids=%s", array_to_char(size, show_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_show_episodes(char* show_id, int limit, int offset) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/shows/%s/episodes?limit=%d&offset=%d", show_id, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_users_saved_shows(int limit, int offset) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/shows?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_shows_for_current_user(int size,char ** show_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/shows?ids=%s", array_to_char(size, show_ids, arr));

  return RestApiPut(url);
}
response Spotify::remove_shows_for_current_user(int size,char ** show_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/shows?ids=%s", array_to_char(size, show_ids, arr));

  return RestApiDelete(url);
}
response Spotify::check_users_saved_shows(int size,char ** show_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/shows/contains?ids=%s", array_to_char(size, show_ids, arr));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_TRACKS
//Tracks
response Spotify::get_track(char* track_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/tracks/%s", track_id);

  return RestApiGet(url);
}
response Spotify::get_several_tracks(int size,char ** track_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/tracks?ids=%s", array_to_char(size, track_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_user_saved_tracks(int limit, int offset) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/tracks?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_tracks_for_current_user(int size,char ** track_ids) {
  char url[] = "https://api.spotify.com/v1/me/tracks";
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, track_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::remove_user_saved_tracks(int size,char ** track_ids) {
  char url[] = "https://api.spotify.com/v1/me/tracks";
  char payload[_max_char_size];
  array_to_json_array(size, track_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_user_saved_tracks(int size,char ** track_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/tracks/contains?ids=%s", array_to_char(size, track_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_tracks_audio_features(int size,char ** track_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/audio-features?ids=%s", array_to_char(size, track_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_track_audio_features(char* track_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/audio-features/%s", track_id);

  return RestApiGet(url);
}
response Spotify::get_track_audio_analysis(char* track_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/audio-analysis/%s", track_id);

  return RestApiGet(url);
}
response Spotify::get_recommendations(recommendations& recom, int limit){
  char url[2000];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/recommendations?limit=%d", limit);

  std::map<char*, char*> char_params;
  std::map<char*, float> float_params;
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
  return RestApiGet(url);
}
bool Spotify::is_valid_value(float param) {
  return param >= 0.0 && param <= 1.0;
}
bool Spotify::is_valid_value(int param) {
  return param >0;
}
void Spotify::populate_float_values(std::map<char*, float>& float_params, recommendations& recom){
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
void Spotify::populate_char_values(std::map<char*, char*>& char_params, recommendations& recom){
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
#ifdef ENABLE_USER
//Users
response Spotify::get_current_user_profile() {
  char url[] = "https://api.spotify.com/v1/me";

  return RestApiGet(url);
}
response Spotify::get_user_top_items(char* type, char* time_range, int limit, int offset) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/top/%s?time_range=%s&limit=%d&offset=%d", type, time_range, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_user_profile(char* user_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/users/%s", user_id);

  return RestApiGet(url);
}
response Spotify::follow_playlist(char* playlist_id, bool is_public) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/followers", playlist_id);
  char payload[100];
  int payload_size = 0;
  snprintf(payload,sizeof(payload), "{\"public\":%s}", is_public ? "true" : "false");
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_playlist(char* playlist_id) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/followers", playlist_id);

  return RestApiDelete(url);
}
response Spotify::get_followed_artists(char* after, char* type, int limit) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/following?type=%s&after=%s&limit=%d", type, after, limit);

  return RestApiGet(url);
}
response Spotify::follow_artists_or_users(char* type,int size, char** artist_user_ids) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/following?type=%s", type);
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, artist_user_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_artists_or_users(char* type,int size, char** artist_user_ids) {
  char url[100];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/following?type=%s", type);
  char payload[_max_char_size];
  array_to_json_array(size, artist_user_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_if_user_follows_artists_or_users(char* type,int size, char** artist_user_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/following/contains?type=%s&ids=%s", type, array_to_char(size, artist_user_ids, arr));

  return RestApiGet(url);
}
response Spotify::check_if_users_follow_playlist(char* playlist_id,int size, char** user_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/followers/contains?ids=%s", playlist_id, array_to_char(size, user_ids, arr));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_SIMPIFIED
//Simplified functions, formatting functions
String Spotify::current_track_name(){
  String track_name = "Something went wrong";
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    track_name = doc["item"]["name"].as<String>();
  }
  return track_name;

}
String Spotify::current_track_id(){
  String track_id = "Something went wrong";
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    track_id = doc["item"]["id"].as<String>();
  }
  return track_id;
}
String Spotify::current_device_id(){
  String device_id = "Something went wrong";
  response data = available_devices();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
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
    JsonDocument doc;
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

char* Spotify::current_device_id(char * device_id){
  response data = available_devices();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    JsonArray devices = doc["devices"].as<JsonArray>();
    for (JsonVariant device : devices) {
      JsonObject deviceObj = device.as<JsonObject>();
      if (deviceObj["is_active"].as<bool>()) {
        snprintf(device_id,sizeof(device_id), "%s", deviceObj["id"].as<String>().c_str());
        break;
      }
    }  
  }
  return device_id;
}
char* Spotify::current_track_name(char * track_name){
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    snprintf(track_name,sizeof(track_name), "%s", doc["item"]["name"].as<String>().c_str());
  }
  return track_name;
}
char* Spotify::current_track_id(char * track_id){
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    snprintf(track_id,sizeof(track_id), "%s", doc["item"]["id"].as<String>().c_str());
  }
  return track_id;
}
char* Spotify::current_artist_names(char * artist_names){
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    JsonArray array = doc["item"]["artists"];
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
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    is_playing = doc["is_playing"].as<bool>();
  }
  return is_playing;
}
bool Spotify::volume_modifyable(){
  bool volume_modifyable = false;
  response data = current_playback_state();
  if((data.status_code>=200)&&(data.status_code<=299)){
    JsonDocument doc;
    deserializeJson(doc,data.reply);
    volume_modifyable = doc["device"]["supports_volume"];
  }
  return volume_modifyable;
}
#endif
char Spotify::convert_id_to_uri(char* id, char* type){
  char uri[_size_of_uri];
  snprintf(uri,_size_of_uri, "spotify:%s:%s", type, id);
  return *uri;
}
char* Spotify::convert_id_to_uri(char* id, char* type,char * uri){
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
  Serial.printf("Reply: %s\n", response_obj.reply.c_str());
}
