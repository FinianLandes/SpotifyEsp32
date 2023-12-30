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
  String TOP_TYPE_ARTIST = "artists";
  String TOP_TYPE_TRACKS = "tracks";
  String GROUP_ALBUM = "album";
  String GROUP_SINGLE = "single";
  String GROUP_APPEARS_ON = "appears_on";
  String GROUP_COMPILATION = "compilation";
  String TIME_RANGE_SHORT = "short_term";
  String TIME_RANGE_MEDIUM = "medium_term";
  String TIME_RANGE_LONG = "long_term";
}


Spotify::Spotify(char* refresh_token, char* redirect_uri, char* client_id, char* client_secret,bool debug_on){
  _retry = 0;
  _refresh_token = refresh_token;
  _redirect_uri = redirect_uri;
  _client_id = client_id;
  _client_secret = client_secret;
  _debug_on = debug_on;
}

//Basic functions
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
    Serial.print("URL: ");
    Serial.println(rest_url);
    Serial.print("GET status: ");
    Serial.println(http_code);
    Serial.print("Reply: ");
    Serial.println(http.getString());
  }
  if ((http_code >=200)&&(http_code<=299)){
    String response = http.getString();
    response_obj.reply = response;
  }
  else if(_retry<=3){
    _retry++;
    if(get_token()){
      response_obj = RestApiGet(rest_url);
    }
  }
  http.end();
  _retry = 0;

  return response_obj;
}
response Spotify::RestApiPut(char rest_url[_size_of_possibly_large_char], String payload){
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
    Serial.print("URL: ");
    Serial.println(rest_url);
    Serial.print("Payload: ");
    Serial.println(payload);
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
        response_obj = RestApiPut(rest_url, payload);
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
    Serial.print("URL: ");
    Serial.println(rest_url);
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.print("POST status: ");
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
        response_obj = RestApiPost(rest_url, payload);
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
    Serial.print("URL: ");
    Serial.println(rest_url);
    Serial.print("Payload: ");
    Serial.println(payload);
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
void Spotify::init_response(response* response_obj){
  response_obj -> status_code = -1;
  response_obj -> reply ="If you see this something went wrong";
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
String Spotify::array_to_string(int size, char** array) {
  String result;
  for (int i = 0; i<size; ++i) {
    result += array[i];
    if (array[i + 1] != nullptr) {
      result += ",";
    }
  }
  
  return result;
}
String Spotify::array_to_json_array(int size,char** array){
  String data;
  DynamicJsonDocument doc(_size_of_possibly_large_char);
  JsonArray json_array = doc.to<JsonArray>();

  for (int i = 0; i<size; ++i) {
    json_array.add(array[i]);
  }

  serializeJson(json_array,data);
  return data;
}
bool Spotify::is_valid_value(float param) {
  return param >= 0.0 && param <= 1.0;
}
bool Spotify::is_valid_value(int param) {
  return param >0;
}
void Spotify::populate_float_values(std::map<String, float>& float_params, recommendations& recom){
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
void Spotify::populate_char_values(std::map<String, String>& char_params, recommendations& recom){
  if (is_valid_value(recom.seed_artists_size)) {
    char_params["seed_artists"] = array_to_string(recom.seed_artists_size, recom.seed_artists);
  }
  if (is_valid_value(recom.seed_genres_size)) {
    char_params["seed_genres"] = array_to_string(recom.seed_genres_size, recom.seed_genres);
  }
  if (is_valid_value(recom.seed_tracks_size)) {
    char_params["seed_tracks"] = array_to_string(recom.seed_tracks_size, recom.seed_tracks);
  }
}
//API functions
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

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_queue(){
  char url[] = "https://api.spotify.com/v1/me/player/queue";
 
  return RestApiGet(url);
}
response Spotify::transfer_playback(String device_id){
  char url[] = "https://api.spotify.com/v1/me/player";
  String payload = "{\"device_ids\":[\"" + device_id + "\"]}";
  
  return RestApiPut(url, payload);
}
response Spotify::seek_to_position(int time_ms){
  String url = "https://api.spotify.com/v1/me/player/seek";
  url += "?position_ms="+String(time_ms);

  return RestApiPut(const_cast<char*>(url.c_str()));
}
response Spotify::repeat_mode(String mode){
  String url = "https://api.spotify.com/v1/me/player/repeat";
  url += "?state="+mode;

  return RestApiPut(const_cast<char*>(url.c_str()));
}
response Spotify::set_volume(int value){
  String url = "https://api.spotify.com/v1/me/player/volume";
  url += "?volume_percent="+String(value);

  return RestApiPut(const_cast<char*>(url.c_str()));
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

  return RestApiPut(const_cast<char*>(url.c_str()));
}
response Spotify::add_to_queue(String context_uri){
  response response_obj;
  init_response(&response_obj);
  String url = "https://api.spotify.com/v1/me/player/queue";
  url += "?uri=" + context_uri;

  return RestApiPost(const_cast<char*>(url.c_str()));
}

//Albums
response Spotify::get_album(String album_id){
  String url = "https://api.spotify.com/v1/albums/";
  url += album_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_albums(int size, char ** album_ids){
  String url = "https://api.spotify.com/v1/albums";
  url  += "?ids=" + array_to_string(size, album_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));

}
response Spotify::get_album_tracks(String album_id, int limit, int offset){
  String url = "https://api.spotify.com/v1/albums/";
  url += album_id + "/tracks?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_users_saved_albums(int limit, int offset){
  String url = "https://api.spotify.com/v1/me/albums";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::save_albums_for_current_user(int size,char ** album_ids){
  char url[] = "https://api.spotify.com/v1/me/albums";

  return RestApiPut(url, array_to_json_array(size, album_ids));
}
response Spotify::remove_users_saved_albums(int size,char ** album_ids){
  char url[] = "https://api.spotify.com/v1/me/albums";
  
  return RestApiDelete(url, array_to_json_array(size, album_ids));
}
response Spotify::check_useres_saved_albums(int size,char ** album_ids){
  String url = "https://api.spotify.com/v1/me/albums/contains";
  url  += "?ids=" + array_to_string(size, album_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_new_releases(String country, int limit, int offset){
  String url = "https://api.spotify.com/v1/browse/new-releases";
  url  += "?country=" + country + "&limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Artists
response Spotify::get_artist(String artist_id){
  String url = "https://api.spotify.com/v1/artists/";
  url  += artist_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_several_artists(int size, char ** artist_ids){
  String url = "https://api.spotify.com/v1/artists";
  url  += "?ids=" + array_to_string(size, artist_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_artist_albums(String artist_id, String include_groups, int limit, int offset){
  String url = "https://api.spotify.com/v1/artists/";
  url  += artist_id +"/albums?include_groups="+ include_groups +"&limit=" + limit +"&offset=" + offset;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_artist_top_tracks(String artist_id, String country){
  String url = "https://api.spotify.com/v1/artists/";
  url  += artist_id + "/top-tracks?market=" + country;

  return RestApiGet(const_cast<char*>(url.c_str()));

}
response Spotify::get_artist_related_artist(String artist_id){
  String url = "https://api.spotify.com/v1/artists/";
  url  += artist_id + "/related-artists";

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Audiobooks
response Spotify::get_audiobook(String audiobook_id){
  String url = "https://api.spotify.com/v1/audiobooks/";
  url  += audiobook_id;

  return RestApiGet(const_cast<char*>(url.c_str()));

}
response Spotify::get_several_audiobooks(int size,char** audiobook_ids){
  String url = "https://api.spotify.com/v1/audiobooks";
  url  += "?ids=" + array_to_string(size, audiobook_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_audiobook_chapters(String audiobook_id, int limit, int offset){
  String url = "https://api.spotify.com/v1/audiobooks/";
  url  += audiobook_id +"/chapters?limit=" +String(limit) +"&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_users_audiobooks(int limit, int offset){
  String url = "https://api.spotify.com/v1/me/audiobooks";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::save_audiobooks_for_current_user(int size,char** audiobook_ids){
  String url = "https://api.spotify.com/v1/me/audiobooks";
  url += "?ids=" + array_to_string(size, audiobook_ids);

  return RestApiPut(const_cast<char*>(url.c_str()));
}
response Spotify::remove_audiobooks_for_current_user(int size,char** audiobook_ids){
  String url = "https://api.spotify.com/v1/me/audiobooks";
  url += "?ids=" + array_to_string(size, audiobook_ids);

  return RestApiDelete(const_cast<char*>(url.c_str()));
}
response Spotify::check_users_saved_audiobooks(int size,char** audiobook_ids){
  String url = "https://api.spotify.com/v1/me/audiobooks/contains";
  url += "?ids=" + array_to_string(size, audiobook_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Categories
response Spotify::get_several_browse_categories(String country, String locale, int limit, int offset){
  String url = "https://api.spotify.com/v1/browse/categories";
  url += "?country=" + country + "&locale=" + locale + "&limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_single_browse_category(String category_id, String country, String locale){
  String url = "https://api.spotify.com/v1/browse/categories/";
  url += category_id+ "?country=" + country + "&locale=" + locale;

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Chapters
response Spotify::get_chapter(String chapter_id){
  String url = "https://api.spotify.com/v1/chapters/";
  url += chapter_id;

  return RestApiGet(const_cast<char*>(url.c_str()));

}
response Spotify::get_several_chapters(int size, char** chapter_ids){
  String url = "https://api.spotify.com/v1/chapters";
  url += +"?ids=" + array_to_string(size, chapter_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Episodes
response Spotify::get_episode(String episode_id){
  String url = "https://api.spotify.com/v1/episodes/";
  url += episode_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_several_episodes(int size,char** episode_ids){
  String url = "https://api.spotify.com/v1/episodes";
  url += "?ids=" + array_to_string(size, episode_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_users_saved_episodes(int limit, int offset){
  String url = "https://api.spotify.com/v1/me/episodes";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::save_episodes_for_current_user(int size,char** episode_ids){
  char url[] = "https://api.spotify.com/v1/me/episodes";

  return RestApiPut(url, array_to_json_array(size, episode_ids));
}
response Spotify::remove_episodes_for_current_user(int size,char** episode_ids){
  char url[] = "https://api.spotify.com/v1/me/episodes";

  return RestApiDelete(url, array_to_json_array(size, episode_ids));
}
response Spotify::check_users_saved_episodes(int size,char** episode_ids){
  String url = "https://api.spotify.com/v1/me/episodes/contains";
  url += "?ids=" + array_to_string(size, episode_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Genres
response Spotify::get_available_genre_seeds(){
  char url[] = "https://api.spotify.com/v1/recommendations/available-genre-seeds";

  return RestApiGet(url);
}

//Markets
response Spotify::get_available_markets(){
  char url[] = "https://api.spotify.com/v1/markets";
  
  return RestApiGet(url);
}

//Playlist
response Spotify::get_playlist(String playlist_id, String fields) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "?fields=" +fields;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::change_playlist_details(String playlist_id, String name, bool is_public, bool is_collaborative, String description) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id; 
  DynamicJsonDocument doc(400);
  if (is_public && is_collaborative){
    is_collaborative = false;

  }
  doc["collaborative"] = is_collaborative;
  doc["name"] = name;
  doc["public"] = is_public;
  doc["description"] = description;

  String payload;
  serializeJson(doc, payload);

  return RestApiPut(const_cast<char*>(url.c_str()), payload);
}
response Spotify::get_playlist_items(String playlist_id, String fields, int limit, int offset) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/tracks?fields=" + fields + "&limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::update_playlist_items(String playlist_id,int size, char** uris, int range_length, int range_start, int insert_before) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/tracks";
  
  DynamicJsonDocument doc(800);
  doc["uris"] = array_to_json_array(size, uris);
  doc["range_start"] = range_start;
  doc["insert_before"] = insert_before;
  doc["range_length"] = range_length;

  String payload;
  serializeJson(doc, payload);

  return RestApiPut(const_cast<char*>(url.c_str()), payload);
}
response Spotify::add_items_to_playlist(String playlist_id, int size, char ** uris, int position) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/tracks";
  DynamicJsonDocument doc(1000);
  doc["uris"] = array_to_json_array(size, uris);
  doc["position"] = position;

  String payload;
  serializeJson(doc, payload);
  return RestApiPost(const_cast<char*>(url.c_str()), payload);
}
response Spotify::remove_playlist_items(String playlist_id,int size, char** uris) { 
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/tracks";

  return RestApiDelete(const_cast<char*>(url.c_str()), array_to_json_array(size, uris));
}
response Spotify::get_current_users_playlists(int limit, int offset) {
  String url = "https://api.spotify.com/v1/me/playlists";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_user_playlists(String user_id, int limit, int offset) {
  String url = "https://api.spotify.com/v1/users/";
  url += user_id + "/playlists?limit=" + String(limit) + "&offset=" +String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::create_playlist(String user_id, String name, bool is_public, bool is_collaborative, String description) {
  String url = "https://api.spotify.com/v1/users/";
  url += user_id + "/playlists";

  if (is_public && is_collaborative){
    is_collaborative = false;
  }

  DynamicJsonDocument doc(256); 
  doc["name"] = name;
  doc["public"] = is_public;
  doc["collaborative"] = is_collaborative;
  doc["description"] = description;
  String payload;
  serializeJson(doc, payload);

  return RestApiPost(const_cast<char*>(url.c_str()),payload);
}
response Spotify::get_featured_playlists(String country, String locale, String timestamp, int limit, int offset) {
  String url = "https://api.spotify.com/v1/browse/featured-playlists";
  url += "?country" + country + "&locale=" + locale + "&timestamp=" + timestamp + "&limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_category_playlists(String category_id, String country, int limit, int offset) {
  String url = "https://api.spotify.com/v1/browse/categories/";
  url += category_id + "/playlists?country" + country + "&limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_playlist_cover_image(String playlist_id) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/images";

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::add_custom_playlist_cover_image(String playlist_id, String data) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/images";

  return RestApiPut(const_cast<char*>(url.c_str()),data);
}

//Search
response Spotify::search(String q, String type, int limit, int offset){
  String url = "https://api.spotify.com/v1/search";
  url += "?q=" + q + "&type=" + type + "&limit=" + String(limit) + "&offset=" + String(offset);
  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Shows
response Spotify::get_show(String show_id) {
  String url = "https://api.spotify.com/v1/shows/";
  url += show_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_several_shows(int size,char ** show_ids) {
  String url = "https://api.spotify.com/v1/shows";
  url += "?ids=" + array_to_string(size, show_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_show_episodes(String show_id, int limit, int offset) {
  String url = "https://api.spotify.com/v1/shows/";
  url += show_id + "/episodes?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_users_saved_shows(int limit, int offset) {
  String url = "https://api.spotify.com/v1/me/shows";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::save_shows_for_current_user(int size,char ** show_ids) {
  String url = "https://api.spotify.com/v1/me/shows";
  url += "?ids=" + array_to_string(size, show_ids);

  return RestApiPut(const_cast<char*>(url.c_str()));
}
response Spotify::remove_shows_for_current_user(int size,char ** show_ids) {
  String url = "https://api.spotify.com/v1/me/shows";
  url += "?ids=" + array_to_string(size, show_ids);

  return RestApiDelete(const_cast<char*>(url.c_str()));
}
response Spotify::check_users_saved_shows(int size,char ** show_ids) {
  String url = "https://api.spotify.com/v1/me/shows/contains";
  url += "?ids=" + array_to_string(size, show_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Tracks
response Spotify::get_track(String track_id) {
  String url = "https://api.spotify.com/v1/tracks/";
  url += track_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_several_tracks(int size,char ** track_ids) {
  String url = "https://api.spotify.com/v1/tracks";
  url += "?ids=" + array_to_string(size, track_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_user_saved_tracks(int limit, int offset) {
  String url = "https://api.spotify.com/v1/me/tracks";
  url += "?limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::save_tracks_for_current_user(int size,char ** track_ids) {
  char url[] = "https://api.spotify.com/v1/me/tracks";

  return RestApiPut(url, array_to_json_array(size, track_ids));
}
response Spotify::remove_user_saved_tracks(int size,char ** track_ids) {
  char url[] = "https://api.spotify.com/v1/me/tracks";

  return RestApiDelete(url, array_to_json_array(size, track_ids));
}
response Spotify::check_user_saved_tracks(int size,char ** track_ids) {
  String url = "https://api.spotify.com/v1/me/tracks/contains";
  url += "?ids=" + array_to_string(size, track_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_tracks_audio_features(int size,char ** track_ids) {
  String url = "https://api.spotify.com/v1/audio-features";
  url += "?ids=" + array_to_string(size, track_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_track_audio_features(String track_id) {
  String url = "https://api.spotify.com/v1/audio-features/";
  url += track_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_track_audio_analysis(String track_id) {
  String url = "https://api.spotify.com/v1/audio-analysis/";
  url += track_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_recommendations(recommendations& recom, int limit){
  String url = "https://api.spotify.com/v1/recommendations";
  url += "?limit=" + String(limit);
  std::map<String, String> char_params;
  std::map<String, float> float_params;
  populate_float_values(float_params, recom);
  populate_char_values(char_params, recom); 

  for (const auto& param : char_params) {
    url += "&" + param.first + "=" + param.second;
  }

  for (const auto& param : float_params) {
    url += "&" + param.first + "=" + String(param.second);
  }

  return RestApiGet(const_cast<char*>(url.c_str()));
}

//Users
response Spotify::get_current_user_profile() {
  char url[] = "https://api.spotify.com/v1/me";

  return RestApiGet(url);
}
response Spotify::get_user_top_items(String type, String time_range, int limit, int offset) {
  String url = "https://api.spotify.com/v1/me/top/";
  url += type +"?time_range=" + time_range + "&limit=" + String(limit) + "&offset=" + String(offset);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::get_user_profile(String user_id) {
  String url = "https://api.spotify.com/v1/users/";
  url += user_id;

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::follow_playlist(String playlist_id, bool is_public) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/followers";
  String payload = is_public ? "true" : "false";

  return RestApiPut(const_cast<char*>(url.c_str()),payload);
}
response Spotify::unfollow_playlist(String playlist_id) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/followers";
 
  return RestApiDelete(const_cast<char*>(url.c_str()));
}
response Spotify::get_followed_artists(String type, String after, int limit) {
  String url = "https://api.spotify.com/v1/me/following";
  url += "?type=" + type + "&after=" + after + "&limit=" + String(limit);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::follow_artists_or_users(String type,int size, char** artist_user_ids) {
  String url = "https://api.spotify.com/v1/me/following";
  url += "?type=" + type;

  return RestApiPut(const_cast<char*>(url.c_str()), array_to_json_array(size, artist_user_ids));
}
response Spotify::unfollow_artists_or_users(String type,int size, char** artist_user_ids) {
  String url = "https://api.spotify.com/v1/me/following";
  url += "?type=" + type;

  return RestApiDelete(const_cast<char*>(url.c_str()), array_to_json_array(size, artist_user_ids));
}
response Spotify::check_if_user_follows_artists_or_users(String type,int size, char** artist_user_ids) {
  String url = "https://api.spotify.com/v1/me/following/contains";
  url += "?type=" + type + "&ids=" + array_to_string(size, artist_user_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}
response Spotify::check_if_users_follow_playlist(String playlist_id,int size, char** user_ids) {
  String url = "https://api.spotify.com/v1/playlists/";
  url += playlist_id + "/followers/contains?ids=" + array_to_string(size, user_ids);

  return RestApiGet(const_cast<char*>(url.c_str()));
}

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
void print_response(response response_obj){
  Serial.print("Status: ");
  Serial.println(response_obj.status_code);
  Serial.print("Reply: ");
  Serial.println(response_obj.reply);
}