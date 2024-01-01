#ifndef SpotifyESP32
#define SpotifyESP32

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <UrlEncode.h>
#include <map>
#include <regex>

namespace Spotify_types{
  extern bool SHUFFLE_ON;//Shuffle on
  extern bool SHUFFLE_OFF;//Shuffle off
  extern char* REPEAT_OFF;//Repeat off
  extern char* REPEAT_TRACK;//Repeat track
  extern char* REPEAT_CONTEXT;//Repeat context
  extern char* TYPE_ALBUM;//URI type album
  extern char* TYPE_ARTIST;//URI type artist
  extern char* TYPE_TRACK;//URI type track
  extern char* TYPE_PLAYLIST;//URI type playlist
  extern char* GROUP_ALBUM;//Get artist's albums include groups album
  extern char* GROUP_SINGLE;//Get artist's albums include groups single
  extern char* GROUP_APPEARS_ON;//Get artist's albums include groups appears on
  extern char* GROUP_COMPILATION;//Get artist's albums include groups compilation
  extern char* TIME_RANGE_SHORT;//4 weeks
  extern char* TIME_RANGE_MEDIUM;//6 months
  extern char* TIME_RANGE_LONG;//Several years
  extern char* TOP_TYPE_ARTIST;//Users top artist's
  extern char* TOP_TYPE_TRACKS;//Users top tracks
  extern int SIZE_OF_URI;//Size to allocate for uri
  extern int SIZE_OF_ID;//Size to allocate for id
};
typedef struct{
  int status_code;
  String reply;
} response;//Response object containing http status code and reply

struct recommendations {
  char** seed_artists;
  int seed_artists_size = 0;
  char** seed_genres;
  int seed_genres_size = 0;
  char** seed_tracks;
  int seed_tracks_size = 0;
  float min_acousticness = -1.0;
  float max_acousticness = -1.0;
  float target_acousticness = -1.0;
  float min_danceability = -1.0;
  float max_danceability = -1.0;
  float target_danceability = -1.0;
  float min_duration_ms = -1.0;
  float max_duration_ms = -1.0;
  float target_duration_ms = -1.0;
  float min_energy = -1.0;
  float max_energy = -1.0;
  float target_energy = -1.0;
  float min_instrumentalness = -1.0;
  float max_instrumentalness = -1.0;
  float target_instrumentalness = -1.0;
  float min_key = -1.0;
  float max_key = -1.0;
  float target_key = -1.0;
  float min_liveness = -1.0;
  float max_liveness = -1.0;
  float target_liveness = -1.0;
  float min_loudness = -1.0;
  float max_loudness = -1.0;
  float target_loudness = -1.0;
  float min_mode = -1.0;
  float max_mode = -1.0;
  float target_mode = -1.0;
  float min_popularity = -1.0;
  float max_popularity = -1.0;
  float target_popularity = -1.0;
  float min_speechiness = -1.0;
  float max_speechiness = -1.0;
  float target_speechiness = -1.0;
  float min_tempo = -1.0;
  float max_tempo = -1.0;
  float target_tempo = -1.0;
  float min_time_signature = -1.0;
  float max_time_signature = -1.0;
  float target_time_signature = -1.0;
  float min_valence = -1.0;
  float max_valence = -1.0;
  float target_valence = -1.0;
};//Fill in the values you want to use for recommendations, at least one seed is required


void print_response(response response_obj);//Print response object

class Spotify {
  public:
    Spotify(const char* refresh_token, const char* redirect_uri,const char* client_id,const char* client_secret, bool debug_on);
    Spotify(const char* refresh_token, const char* redirect_uri,const char* client_id,const char* client_secret, bool debug_on, int max_num_retry);
    Spotify(const char* refresh_token, const char* redirect_uri,const char* client_id,const char* client_secret); 
    //Player
    /*@brief Get information about the user's current playback state, including track, track progress, and active device.
    * @return response object containing http status code and reply
    */
    response currently_playing();
    /*@brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. If no     context is provided, the user's currently playing context (e.g. album, playlist, etc.) is the target.
    * @param context_uri Spotify URI of the context to play (Required)
    * @param offset Indicates from where in the context playback should start, Only works with albums or Playlists(Optional)
    * @param position_ms Indicates from what position in the context playback should start in milliseconds(Optional)
    * @param device_id Id of the device this command is targeting (Optional)
    * @return response object containing http status code and reply
    */
    response start_resume_playback(char* context_uri, int offset = 0, int position_ms = 0, char* device_id = nullptr);
    /*@brief Start or resume playback.
    * @param size Number of uris in uris array
    * @param uris Array of Spotify URIs of the tracks to play
    * @param device_id Id of the device this command is targeting (Optional)
    * @return response object containing http status code and reply
    */
    response start_resume_playback(int size, char ** uris ,char* device_id = nullptr);
    /*@brief Start or resume playback on provided device
    * @param device_id Id of the device this command is targeting(Optional)
    * @return response object containing http status code and reply
    */
    response start_resume_playback(char* device_id = nullptr);
    /*@brief Pause playback on Spotify
    * @return response object containing http status code and reply
    */
    response pause_playback();
    /*@brief Skip to next track
    * @return response object containing http status code and reply
    */
    response skip();
    /*@brief Skip to previous track
    * @return response object containing http status code and reply
    */
    response previous();
    /*@brief get information about the user's available devices
    * @return response object containing http status code and reply
    */
    response available_devices();
    /*@brief get information about the user's current playback state, including track, track progress, and active device, shuffle etc.
    * @return response object containing http status code and reply
    */
    response current_playback_state();
    /*@brief Get recently played tracks
    * @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    */
    response recently_played_tracks(int limit = 10);
    /*@brief Seek to position of current context
    * @param time_ms Position in milliseconds to seek to, if the value is greater than the length of the track the player will skip to the next track
    * @return response object containing http status code and reply
    */
    response seek_to_position(int time_ms);
    /*@brief get users queue, response can be empty or containing episode or track objects
    * @return response object containing http status code and reply
    */
    response get_queue();
    /*@Brief Set repeat mode, allowed values are REPEAT_OFF, REPEAT_TRACK, REPEAT_CONTEXT
    * @param mode Repeat mode
    * @return response object containing http status code and reply
    */
    response repeat_mode(char* mode);
    /*@Brief Set shuffle mode, allowed values are SHUFFLE_ON, SHUFFLE_OFF
    * @param mode Shuffle mode
    * @return response object containing http status code and reply
    */
    response shuffle(bool mode);
    /*@Brief Transfer playback to another device
    * @param device_id Id of the device this command is targeting
    * @return response object containing http status code and reply
    */
    response transfer_playback(char* device_id);
    /*@Brief Set volume, does not work with all devices(eg. does not work on Phones)
    * @param value Volume value between 0 and 100
    * @return response object containing http status code and reply
    */
    response set_volume(int value);
    /*@Brief Add context to queue 
    * @param context_uri Spotify URI of the context to add to queue
    */
    response add_to_queue(char* context_uri);
    //Albums
    response get_album(char* album_id);
    response get_albums(int size,  char** album_ids);
    response get_album_tracks(char* album_id, int limit = 10, int offset = 0);
    response get_users_saved_albums(int limit = 10, int offset = 0);
    response save_albums_for_current_user(int size,  char** album_ids);
    response remove_users_saved_albums(int size,  char** album_ids);
    response check_useres_saved_albums(int size,  char** album_ids);
    response get_new_releases(char* country, int limit = 10, int offset = 0);
    //Artists
    response get_artist(char* artist_id);
    response get_several_artists(int size,  char** artist_ids);
    response get_artist_albums(char* artist_id,int size_groups, char** include_groups, int limit = 10, int offset = 0);
    response get_artist_top_tracks(char* artist_id, char* country);
    response get_artist_related_artist(char* artist_id);
    //Audiobooks (Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    response get_audiobook(char* audiobook_id);
    response get_several_audiobooks(int size,  char** audiobook_ids);
    response get_audiobook_chapters(char* audiobook_id, int limit = 10, int offset = 0);
    response get_users_audiobooks(int limit = 10, int offset = 0);
    response save_audiobooks_for_current_user(int size,  char** audiobook_ids);
    response remove_audiobooks_for_current_user(int size,  char** audiobook_ids);
    response check_users_saved_audiobooks(int size,  char** audiobook_ids);
    //Categories
    response get_several_browse_categories(char* country, char* locale, int limit = 10, int offset = 0);
    response get_single_browse_category(char* category_id, char* country, char* locale);
    //Chapters (Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    response get_chapter(char* chapter_id);
    response get_several_chapters(int size,  char** chapter_ids);
    //Episodes
    response get_episode(char* episode_id);
    response get_several_episodes(int size,  char** episode_ids);
    response get_users_saved_episodes(int limit = 10, int offset = 0);
    response save_episodes_for_current_user(int size,  char** episode_ids);
    response remove_episodes_for_current_user(int size,  char** episode_ids);
    response check_users_saved_episodes(int size,  char** episode_ids);
    //Genres
    response get_available_genre_seeds();
    //Markets
    response get_available_markets();
    //Playlists
    response get_playlist(char* playlist_id, char* fields);
    response change_playlist_details(char* playlist_id, char* name, bool is_public, bool is_collaborative, char* description);
    response get_playlist_items(char* playlist_id, char* fields, int limit = 10, int offset = 0);
    response update_playlist_items(char* playlist_id, int size,  char** uris = nullptr, int range_length = 1, int range_start = 0, int insert_before = 1 );
    response add_items_to_playlist(char* playlist_id, int size,  char** uris, int position = 0);
    response remove_playlist_items(char* playlist_id, int size,  char** uris);
    response get_current_users_playlists(int limit = 10, int offset = 0);
    response get_user_playlists(char* user_id,int limit = 10, int offset = 0);
    response create_playlist(char* user_id, char* name, bool is_public, bool is_collaborative, char* description);
    response get_featured_playlists(char* country, char* locale, char* timestamp,int limit = 10, int offset = 0);
    response get_category_playlists(char* category_id, char* country, int limit = 10, int offset = 0);
    response get_playlist_cover_image(char* playlist_id);
    response add_custom_playlist_cover_image(char* playlist_id, char* data);
    //Search
    response search(char* q, char* type, int limit = 10, int offset = 0);
    //Shows
    response get_show(char* show_id);
    response get_several_shows(int size,  char** show_ids);
    response get_show_episodes(char* show_id, int limit = 10, int offset = 0);
    response get_users_saved_shows(int limit = 10, int offset = 0);
    response save_shows_for_current_user(int size,  char** show_ids);
    response remove_shows_for_current_user(int size,  char** show_ids);
    response check_users_saved_shows(int size,  char** show_ids);
    //Tracks
    response get_track(char* track_id);
    response get_several_tracks(int size,  char** track_ids);
    response get_user_saved_tracks(int limit = 10, int offset = 0);
    response save_tracks_for_current_user(int size,  char** track_ids);
    response remove_user_saved_tracks(int size,  char** track_ids);
    response check_user_saved_tracks(int size,  char** track_ids);
    response get_tracks_audio_features(int size,  char** track_ids);
    response get_track_audio_features(char* track_id);
    response get_track_audio_analysis(char* track_id);
    response get_recommendations(recommendations& recom, int limit = 10);
    
    //Users
    response get_current_user_profile();
    response get_user_top_items(char* type, char* time_range = "medium_term", int limit = 10, int offset = 0);
    response get_user_profile(char* user_id);
    response follow_playlist(char* playlist_id, bool is_public);
    response unfollow_playlist(char* playlist_id);
    response get_followed_artists(char* after, char* type = "artist", int limit = 10);
    response follow_artists_or_users(char* type, int size,  char** artist_user_ids);
    response unfollow_artists_or_users(char* type, int size,  char** artist_user_ids);
    response check_if_user_follows_artists_or_users(char* type, int size,  char** artist_user_ids);
    response check_if_users_follow_playlist(char* playlist_id, int size,  char** user_ids);

    //Simplified versions of the above
    String current_track_name();
    String current_track_id();
    String current_device_id();
    String current_artist_names();
    char* current_device_id(char* device_id);
    
    //String convert_id_to_uri(String id, String type);
    char convert_id_to_uri(char* id, char* type);
    char* convert_id_to_uri(char* id, char* type, char* uri); 
    bool is_playing();

  private:
  
    static const int _max_num_items = 20;//Number of items to be sent in one request
    static const int _max_char_size = 35*_max_num_items + 150;// 35 beeing the size of a uri + comma + 150 as buffer for url etc.
    static const int _size_of_uri = 45;
    static const int _size_of_id = 25;
    int _max_num_retry = 3;//Max number of retries for a request
    const char* _refresh_token;
    const char* _redirect_uri;
    const char* _client_id;
    const char* _client_secret;
    int _retry;
    bool _debug_on;
    String _access_token;
    bool get_token();//Get access token
    void init_response(response* response_obj);//Initialize response object

    response RestApiPut(char* rest_url, int payload_size = 0, char* payload = nullptr);
    response RestApiPost(char* rest_url, int payload_size = 0, char* payload = nullptr);
    response RestApiDelete(char* rest_url, char* payload = nullptr);
    response RestApiGet(char* rest_url); 

    char* array_to_char(int size,  char** array);//Convert array of chars to one comma separated char
    void array_to_json_array(int size,  char** array, char* data, int data_size = _max_char_size);//Convert array of chars to one json array
    bool is_valid_value(float param);//Check if recommendation value is valid
    bool is_valid_value(int param);//Check if recommendation value is valid
    void populate_char_values(std::map<char*, char*>& map, recommendations& recom);//Populate recommendation char values
    void populate_float_values(std::map<char*, float>& map, recommendations& recom);//Populate recommendation float values
    const char * extract_endpoint(char* rest_url);//Extract endpoint from url with regex



    const char* _spotify_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"\
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"\
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"\
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"\
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"\
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"\
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"\
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"\
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"\
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"\
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"\
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"\
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"\
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"\
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"\
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"\
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"\
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"\
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"\
    "-----END CERTIFICATE-----\n";
};
#endif