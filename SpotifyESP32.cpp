#include "SpotifyESP32.h"

namespace Spotify_types{
  bool SHUFFLE_ON = true;
  bool SHUFFLE_OFF = false;
  char* REPEAT_OFF = "off";
  char* REPEAT_TRACK = "track";
  char* REPEAT_CONTEXT = "context";
  char* TYPE_ALBUM = "album";
  char* TYPE_ARTIST = "artist";
  char* TYPE_TRACK = "track";
  char* TYPE_PLAYLIST = "playlist";
  char* TOP_TYPE_ARTIST = "artists";
  char* TOP_TYPE_TRACKS = "tracks";
  char* GROUP_ALBUM = "album";
  char* GROUP_SINGLE = "single";
  char* GROUP_APPEARS_ON = "appears_on";
  char* GROUP_COMPILATION = "compilation";
  char* TIME_RANGE_SHORT = "short_term";
  char* TIME_RANGE_MEDIUM = "medium_term";
  char* TIME_RANGE_LONG = "long_term";
  int SIZE_OF_ID = 40;
  int SIZE_OF_URI = 50;
}


Spotify::Spotify(const char* refresh_token, const char* redirect_uri, const char* client_id,const char* client_secret, bool debug_on, int max_num_retry){
  _retry = 0;
  _refresh_token = refresh_token;
  _redirect_uri = redirect_uri;
  _client_id = client_id;
  _client_secret = client_secret;
  _debug_on = debug_on;
  if(max_num_retry>0){
    _max_num_retry = max_num_retry;
  }
  else{
    _max_num_retry = 1;
  }
}
//Basic functions
response Spotify::RestApiPut(char* rest_url,int payload_size, char* payload){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  http.addHeader("content-Length", String(payload_size));
  int http_code=http.PUT(payload);

  response_obj.status_code = http_code;
  String reply = "";
  DynamicJsonDocument doc(2000);

  if(http.getSize()>0){
    reply = http.getString();
    deserializeJson(doc, reply);
  }

  if(_debug_on){
    const char* endpoint = extract_endpoint(rest_url);
    Serial.printf("PUT \"%s\" status: ", endpoint);
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(reply);
  }
  if ((http_code >= 200)&&(http_code <= 299)){
    response_obj.reply = "Success";    
  }
  else if(_retry<=_max_num_retry){
    String message = doc["error"]["message"].as<String>();
    if(message == "Only valid bearer authentication supported"){
      _retry++;
      if(get_token()){
        response_obj = RestApiPut(rest_url,payload_size, payload);
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
response Spotify::RestApiGet(char* rest_url){
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
    const char* endpoint = extract_endpoint(rest_url);
    Serial.printf("GET \"%s\" status: ", endpoint);
    Serial.println(http_code);
    Serial.print("Reply: ");
    Serial.println(http.getString());
  }
  if ((http_code >=200)&&(http_code<=299)){
    String response = http.getString();
    response_obj.reply = response;
  }
  else if(_retry<=_max_num_retry){
    _retry++;
    if(get_token()){
      response_obj = RestApiGet(rest_url);
    }
  }
  http.end();
  _retry = 0;

  return response_obj;
}
response Spotify::RestApiPost(char* rest_url,int payload_size, char* payload){
  response response_obj;
  init_response(&response_obj);

  HTTPClient http;
  http.begin(rest_url,_spotify_root_ca);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization","Bearer "+String(_access_token));

  
  http.addHeader("content-Length", String(payload_size));
  int http_code=http.POST(payload);

  response_obj.status_code = http_code;
  String reply = "";
  DynamicJsonDocument doc(2000);

  if(http.getSize()>0){
    reply = http.getString();
    deserializeJson(doc, reply);
  }

  if(_debug_on){
    const char* endpoint = extract_endpoint(rest_url);
    Serial.printf("POST \"%s\" status: ", endpoint);
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(reply);
  }
  if ((http_code >= 200)&&(http_code <= 299)){
    response_obj.reply = "Success";    
  }
  else if(_retry<=_max_num_retry){
    String message = doc["error"]["message"].as<String>();
    if(message == "Only valid bearer authentication supported"){
      _retry++;
      if(get_token()){
        response_obj = RestApiPost(rest_url,payload_size, payload);
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
response Spotify::RestApiDelete(char* rest_url, char* payload){
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
    const char* endpoint = extract_endpoint(rest_url);
    Serial.printf("DELETE \"%s\" status: ", endpoint);
    Serial.println(http_code);
    Serial.print(" Reply: ");
    Serial.println(reply);
  }

  if ((http_code >= 200) && (http_code <= 299)) {
    response_obj.reply = "Success";
  } else if (_retry <= _max_num_retry) {
    String message = doc["error"]["message"].as<String>();
    if (message == "Only valid bearer authentication supported") {
      _retry++;
      if (get_token()) {
        response_obj = RestApiDelete(rest_url, payload); 
      }
    } else {
      response_obj.reply = message;
    }
  }

  http.end();
  _retry = 0;

  return response_obj;

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
    Serial.print("POST \"token\" status: ");
    Serial.println(http_code);
  }
  if ((http_code >=200)&&(http_code<=299)) {
    String response = http.getString();
    StaticJsonDocument<500> doc;
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
void Spotify::init_response(response* response_obj){
  response_obj -> status_code = -1;
  response_obj -> reply ="If you see this something went wrong";
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
void Spotify::array_to_json_array(int size,char** array, char* data, int data_size){
  DynamicJsonDocument doc(_max_char_size);
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
  char url[] = "https://api.spotify.com/v1/me/player/play";
  if(device_id != nullptr){
    sprintf(url, "https://api.spotify.com/v1/me/player/play?device_id=%s", device_id);
  }
  char payload[_max_char_size]; 
  int payload_size = 0;
  if(context_uri != nullptr){
    if (strncmp(context_uri, "spotify:track:", 14) == 0) {
      sprintf(payload, "{\"uris\":[\"%s\"]}", context_uri);
      
    } else {
      sprintf(payload, "{\"context_uri\":\"%s\",\"offset\":{\"position\":%d},\"position_ms\":%d}",context_uri, offset, position_ms);
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
  char url[] = "https://api.spotify.com/v1/me/player/play";
  if(device_id != nullptr){
    sprintf(url, "https://api.spotify.com/v1/me/player/play?device_id=%s", device_id);
  }
  char payload[_max_char_size]; 
  int payload_size = 0;
  char arr[_max_char_size];
  array_to_json_array(size, uris, arr);
  sprintf(payload, "{\"uris\":%s}", arr);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::start_resume_playback(char* device_id){
  char url[100];
  if(device_id != nullptr){
    sprintf(url, "https://api.spotify.com/v1/me/player/play?device_id=%s", device_id);
  }
  else{
    sprintf(url, "https://api.spotify.com/v1/me/player/play");
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
  sprintf(url, "https://api.spotify.com/v1/me/player/recently-played?limit=%d", limit);

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
  sprintf(payload, "{\"device_ids\":[\"%s\"]}", device_id);
  payload_size = strlen(payload);
  return RestApiPut(url,payload_size, payload);
}
response Spotify::seek_to_position(int time_ms){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/player/seek?position_ms=%d", time_ms);

  return RestApiPut(url);
}
response Spotify::repeat_mode(char* mode){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/player/repeat?state=%s", mode);

  return RestApiPut(url);
}
response Spotify::set_volume(int value){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/player/volume?volume_percent=%d", value);

  return RestApiPut(url);
}
response Spotify::shuffle(bool mode){
  char state[6]; 
  sprintf(state, "%s", mode ? "true" : "false");

  char url[100]; 
  sprintf(url, "https://api.spotify.com/v1/me/player/shuffle?state=%s", state);

  return RestApiPut(url);
}
response Spotify::add_to_queue(char* context_uri){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/player/queue?uri=%s", context_uri);

  return RestApiPost(url);
}
#endif
#ifdef ENABLE_ALBUM
//Albums
response Spotify::get_album(char* album_id){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/albums/%s", album_id);
  
  return RestApiGet(url);
}
response Spotify::get_albums(int size, char ** album_ids){
  char url[_max_char_size];
  char arr[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/albums?ids=%s", array_to_char(size, album_ids,arr));
  
  return RestApiGet(url);
}
response Spotify::get_album_tracks(char* album_id, int limit, int offset){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/albums/%s/tracks?limit=%d&offset=%d", album_id, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_users_saved_albums(int limit, int offset){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/albums?limit=%d&offset=%d", limit, offset);

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
  sprintf(url, "https://api.spotify.com/v1/me/albums/contains?ids=%s", array_to_char(size, album_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_new_releases(int limit, int offset, char* country){
  char url[120];
  if(country == nullptr){
    sprintf(url, "https://api.spotify.com/v1/browse/new-releases?limit=%d&offset=%d", limit, offset);
  }
  else{
    sprintf(url, "https://api.spotify.com/v1/browse/new-releases?country=%s&limit=%d&offset=%d", country, limit, offset);
  }

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_ARTIST
//Artists
response Spotify::get_artist(char* artist_id){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/artists/%s", artist_id);

  return RestApiGet(url);
}
response Spotify::get_several_artists(int size, char ** artist_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/artists?ids=%s", array_to_char(size, artist_ids));

  return RestApiGet(url);
}
response Spotify::get_artist_albums(char* artist_id,int size_groups, char** include_groups, int limit, int offset){
  char url[200];
  sprintf(url, "https://api.spotify.com/v1/artists/%s/albums?include_groups=%s&limit=%d&offset=%d", artist_id, array_to_char(size_groups,include_groups), limit, offset);  

  return RestApiGet(url);
}
response Spotify::get_artist_top_tracks(char* artist_id, char* country){
  char url[100];
  if(country == nullptr){
    sprintf(url, "https://api.spotify.com/v1/artists/%s/top-tracks", artist_id);
  }
  else{
    sprintf(url, "https://api.spotify.com/v1/artists/%s/top-tracks?country=%s", artist_id, country);
  }

  return RestApiGet(url);
}
response Spotify::get_artist_related_artist(char* artist_id){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/artists/%s/related-artists", artist_id);

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_AUDIOBOOKS
//Audiobooks
response Spotify::get_audiobook(char* audiobook_id){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/audiobooks/%s", audiobook_id);

  return RestApiGet(url);
}
response Spotify::get_several_audiobooks(int size, char** audiobook_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/audiobooks?ids=%s", array_to_char(size, audiobook_ids));

  return RestApiGet(url);
}
response Spotify::get_audiobook_chapters(char* audiobook_id, int limit, int offset){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/audiobooks/%s/chapters?limit=%d&offset=%d", audiobook_id, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_users_audiobooks(int limit, int offset){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/audiobooks?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_audiobooks_for_current_user(int size, char** audiobook_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/audiobooks?ids=%s", array_to_char(size, audiobook_ids));

  return RestApiPut(url);
}
response Spotify::remove_audiobooks_for_current_user(int size, char** audiobook_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/audiobooks?ids=%s", array_to_char(size, audiobook_ids));

  return RestApiDelete(url);
}
response Spotify::check_users_saved_audiobooks(int size, char** audiobook_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/audiobooks/contains?ids=%s", array_to_char(size, audiobook_ids));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_CATEGORIES
//Categories
response Spotify::get_several_browse_categories(char* country, char* locale, int limit, int offset){
  char url[200];
  sprintf(url, "https://api.spotify.com/v1/browse/categories?country=%s&locale=%s&limit=%d&offset=%d", country, locale, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_single_browse_category(char* category_id, char* country, char* locale){
  char url[150];
  sprintf(url, "https://api.spotify.com/v1/browse/categories/%s?country=%s&locale=%s", category_id, country, locale);

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_CHAPTERS
//Chapters
response Spotify::get_chapter(char* chapter_id){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/chapters/%s", chapter_id);

  return RestApiGet(url);
}
response Spotify::get_several_chapters(int size, char** chapter_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/chapters?ids=%s", array_to_char(size, chapter_ids));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_EPISODES
//Episodes
response Spotify::get_episode(char* episode_id){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/episodes/%s", episode_id);

  return RestApiGet(url);
}
response Spotify::get_several_episodes(int size,char** episode_ids){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/episodes?ids=%s", array_to_char(size, episode_ids));

  return RestApiGet(url);
}
response Spotify::get_users_saved_episodes(int limit, int offset){
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/episodes?limit=%d&offset=%d", limit, offset);

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
  sprintf(url, "https://api.spotify.com/v1/me/episodes/contains?ids=%s", array_to_char(size, episode_ids));

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
response Spotify::get_playlist(char* playlist_id, char* fields) {
  char url[200];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s?fields=%s", playlist_id, fields);

  return RestApiGet(url);
}
response Spotify::change_playlist_details(char* playlist_id, char* name, bool is_public, bool is_collaborative, char* description) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s", playlist_id);

  DynamicJsonDocument doc(400);
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
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/tracks?fields=%s&limit=%d&offset=%d", playlist_id, fields, limit, offset);

  return RestApiGet(url);
}
response Spotify::update_playlist_items(char* playlist_id,int size, char** uris, int range_length, int range_start, int insert_before) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);
  
  DynamicJsonDocument doc(_max_char_size);
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
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);
  
  DynamicJsonDocument doc(1000);
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
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);

  char* payload = array_to_char(size, uris);
  
  
  return RestApiDelete(url, payload);
}
response Spotify::get_current_users_playlists(int limit, int offset) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/playlists?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::get_user_playlists(char* user_id, int limit, int offset) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/users/%s/playlists?limit=%d&offset=%d", user_id, limit, offset);
  
  return RestApiGet(url);
}
response Spotify::create_playlist(char* user_id, char* name, bool is_public, bool is_collaborative, char* description) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/users/%s/playlists", user_id);

  if (is_public && is_collaborative){
    is_collaborative = false;
  }

  DynamicJsonDocument doc(256); 
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
response Spotify::get_featured_playlists(char* country, char* locale, char* timestamp, int limit, int offset) {
  char url[200];
  sprintf(url, "https://api.spotify.com/v1/browse/featured-playlists?country=%s&locale=%s&timestamp=%s&limit=%d&offset=%d", country, locale, timestamp, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_category_playlists(char* category_id, char* country, int limit, int offset) {
  char url[200];
  sprintf(url, "https://api.spotify.com/v1/browse/categories/%s/playlists?country=%s&limit=%d&offset=%d", category_id, country, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_playlist_cover_image(char* playlist_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/images", playlist_id);

  return RestApiGet(url);
}
response Spotify::add_custom_playlist_cover_image(char* playlist_id, char* data) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/images", playlist_id);  
  int payload_size = strlen(data);

  return RestApiPut(url, payload_size, data);
}
#endif
#ifdef ENABLE_SEARCH
//Search
response Spotify::search(char* q, char* type, int limit, int offset){
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/search?q=%s&type=%s&limit=%d&offset=%d", q, type, limit, offset);

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_SHOWS
//Shows
response Spotify::get_show(char* show_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/shows/%s", show_id);

  return RestApiGet(url);
}
response Spotify::get_several_shows(int size,char ** show_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/shows?ids=%s", array_to_char(size, show_ids));

  return RestApiGet(url);
}
response Spotify::get_show_episodes(char* show_id, int limit, int offset) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/shows/%s/episodes?limit=%d&offset=%d", show_id, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_users_saved_shows(int limit, int offset) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/shows?limit=%d&offset=%d", limit, offset);

  return RestApiGet(url);
}
response Spotify::save_shows_for_current_user(int size,char ** show_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/shows?ids=%s", array_to_char(size, show_ids));

  return RestApiPut(url);
}
response Spotify::remove_shows_for_current_user(int size,char ** show_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/shows?ids=%s", array_to_char(size, show_ids));

  return RestApiDelete(url);
}
response Spotify::check_users_saved_shows(int size,char ** show_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/shows/contains?ids=%s", array_to_char(size, show_ids));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_TRACKS
//Tracks
response Spotify::get_track(char* track_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/tracks/%s", track_id);

  return RestApiGet(url);
}
response Spotify::get_several_tracks(int size,char ** track_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/tracks?ids=%s", array_to_char(size, track_ids));

  return RestApiGet(url);
}
response Spotify::get_user_saved_tracks(int limit, int offset) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/tracks?limit=%d&offset=%d", limit, offset);

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
  sprintf(url, "https://api.spotify.com/v1/me/tracks/contains?ids=%s", array_to_char(size, track_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_tracks_audio_features(int size,char ** track_ids) {
  char url[_max_char_size];
  char arr[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/audio-features?ids=%s", array_to_char(size, track_ids, arr));

  return RestApiGet(url);
}
response Spotify::get_track_audio_features(char* track_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/audio-features/%s", track_id);

  return RestApiGet(url);
}
response Spotify::get_track_audio_analysis(char* track_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/audio-analysis/%s", track_id);

  return RestApiGet(url);
}
response Spotify::get_recommendations(recommendations& recom, int limit){
  char url[2000];
  sprintf(url, "https://api.spotify.com/v1/recommendations?limit=%d", limit);

  std::map<char*, char*> char_params;
  std::map<char*, float> float_params;
  populate_float_values(float_params, recom);
  populate_char_values(char_params, recom); 

  for (const auto& param : char_params) {
    char value[100];
    sprintf(value, "&%s%s",param.first, param.second);
    strcat(url, value);
  }
  for(const auto& param : float_params){
    char value[100];
    sprintf(value, "&%s=%f",param.first, param.second);
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
  sprintf(url, "https://api.spotify.com/v1/me/top/%s?time_range=%s&limit=%d&offset=%d", type, time_range, limit, offset);

  return RestApiGet(url);
}
response Spotify::get_user_profile(char* user_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/users/%s", user_id);

  return RestApiGet(url);
}
response Spotify::follow_playlist(char* playlist_id, bool is_public) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/followers", playlist_id);
  char payload[100];
  int payload_size = 0;
  sprintf(payload, "{\"public\":%s}", is_public ? "true" : "false");
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_playlist(char* playlist_id) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/followers", playlist_id);

  return RestApiDelete(url);
}
response Spotify::get_followed_artists(char* type, char* after, int limit) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/following?type=%s&after=%s&limit=%d", type, after, limit);

  return RestApiGet(url);
}
response Spotify::follow_artists_or_users(char* type,int size, char** artist_user_ids) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/following?type=%s", type);
  char payload[_max_char_size];
  int payload_size = 0;
  array_to_json_array(size, artist_user_ids, payload);
  payload_size = strlen(payload);

  return RestApiPut(url, payload_size, payload);
}
response Spotify::unfollow_artists_or_users(char* type,int size, char** artist_user_ids) {
  char url[100];
  sprintf(url, "https://api.spotify.com/v1/me/following?type=%s", type);
  char payload[_max_char_size];
  array_to_json_array(size, artist_user_ids, payload);

  return RestApiDelete(url, payload);
}
response Spotify::check_if_user_follows_artists_or_users(char* type,int size, char** artist_user_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/me/following/contains?type=%s&ids=%s", type, array_to_char(size, artist_user_ids));

  return RestApiGet(url);
}
response Spotify::check_if_users_follow_playlist(char* playlist_id,int size, char** user_ids) {
  char url[_max_char_size];
  sprintf(url, "https://api.spotify.com/v1/playlists/%s/followers/contains?ids=%s", playlist_id, array_to_char(size, user_ids));

  return RestApiGet(url);
}
#endif
#ifdef ENABLE_SIMPIFIED
//Simplified functions, formatting functions
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

char* Spotify::current_device_id(char * device_id){
  response data = available_devices();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(2000);
    deserializeJson(doc,data.reply);
    JsonArray devices = doc["devices"].as<JsonArray>();
    for (JsonVariant device : devices) {
      JsonObject deviceObj = device.as<JsonObject>();

      if (deviceObj["is_active"].as<bool>()) {
        sprintf(device_id, "%s", deviceObj["id"].as<String>().c_str());
        break;
      }
    }  
  }
  return device_id;
}
char* Spotify::current_track_name(char * track_name){
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    sprintf(track_name, "%s", doc["item"]["name"].as<String>().c_str());
  }
  return track_name;
}
char* Spotify::current_track_id(char * track_id){
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    sprintf(track_id, "%s", doc["item"]["id"].as<String>().c_str());
  }
  return track_id;
}
char* Spotify::current_artist_names(char * artist_names){
  response data = currently_playing();
  if((data.status_code>=200)&&(data.status_code<=299)){
    DynamicJsonDocument doc(10000);
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
    DynamicJsonDocument doc(10000);
    deserializeJson(doc,data.reply);
    is_playing = doc["is_playing"].as<bool>();
  }
  return is_playing;
}
#endif
char Spotify::convert_id_to_uri(char* id, char* type){
  char uri[45];
  sprintf(uri, "spotify:%s:%s", type, id);
  return *uri;
}
char* Spotify::convert_id_to_uri(char* id, char* type,char * uri){
  sprintf(uri, "spotify:%s:%s", type, id);
  return uri;
}
void print_response(response response_obj){
  Serial.print("Status: ");
  Serial.println(response_obj.status_code);
  /*Serial.print("Reply: ");
  Serial.println(response_obj.reply);*/
} 