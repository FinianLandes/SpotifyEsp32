#ifndef SpotifyESP32
#define SpotifyESP32
//#define ENABLE_PLAYER
#define ENABLE_ALBUM
//#define ENABLE_ARTIST
//#define ENABLE_AUDIOBOOKS
//#define ENABLE_CATEGORIES
//#define ENABLE_CHAPTERS
//#define ENABLE_EPISODES
//#define ENABLE_GENRES
//#define ENABLE_MARKETS
//#define ENABLE_PLAYLISTS
//#define ENABLE_SEARCH
//#define ENABLE_SHOWS
//define ENABLE_TRACKS
//#define ENABLE_USERS
//#define ENABLE_SIMPIFIED //Player needs to be enabled for this to work


#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <UrlEncode.h>
#include <map>
#include <regex>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



namespace Spotify_types{
  extern const bool SHUFFLE_ON;//Shuffle on
  extern const bool SHUFFLE_OFF;//Shuffle off
  extern const char* REPEAT_OFF;//Repeat off
  extern const char* REPEAT_TRACK;//Repeat track
  extern const char* REPEAT_CONTEXT;//Repeat context
  extern const char* TYPE_ALBUM;//URI type album
  extern const char* TYPE_ARTIST;//URI type artist
  extern const char* TYPE_TRACK;//URI type track
  extern const char* TYPE_PLAYLIST;//URI type playlist
  extern const char* GROUP_ALBUM;//Get artist's albums include groups album
  extern const char* GROUP_SINGLE;//Get artist's albums include groups single
  extern const char* GROUP_APPEARS_ON;//Get artist's albums include groups appears on
  extern const char* GROUP_COMPILATION;//Get artist's albums include groups compilation
  extern const char* TIME_RANGE_SHORT;//4 weeks
  extern const char* TIME_RANGE_MEDIUM;//6 months
  extern const char* TIME_RANGE_LONG;//Several years
  extern const char* TOP_TYPE_ARTIST;//Users top artist's
  extern const char* TOP_TYPE_TRACKS;//Users top tracks
  extern const int SIZE_OF_URI;//Size to allocate for uri
  extern const int SIZE_OF_ID;//Size to allocate for id
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
    ///@brief Constructor for Spotify object
    ///@param refresh_token Refresh token from Spotify(Required)
    ///@param redirect_uri Redirect uri from Spotify(Required)
    ///@param client_id Client id from Spotify(Required)
    ///@param client_secret Client secret from Spotify(Required)
    ///@param debug_on Debug mode on or off(default off)
    ///@param max_num_retry Max number of retries for a request(default 3)
    Spotify(const char* refresh_token, const char* redirect_uri,const char* client_id,const char* client_secret, bool debug_on = false, int max_num_retry =3);
    //Player
  #ifdef ENABLE_PLAYER
    ///@brief Get information about the user's current playback state, including track, track progress, and active device.
    ///@return response object containing http status code and reply
    response currently_playing();
    ///@brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. If no     context is provided, the user's currently playing context (e.g. album, playlist, etc.) is the target.
    ///@param context_uri Spotify URI of the context to play (Required)
    ///@param offset Indicates from where in the context playback should start, Only works with albums or Playlists(Optional)
    ///@param position_ms Indicates from what position in the context playback should start in milliseconds(Optional)
    ///@param device_id Id of the device this command is targeting (Optional)
    ///@return response object containing http status code and reply
    response start_resume_playback(char* context_uri, int offset = 0, int position_ms = 0, char* device_id = nullptr);
    ///@brief Start or resume playback.
    ///@param size Number of uris in uris array
    ///@param uris Array of Spotify URIs of the tracks to play
    ///@param device_id Id of the device this command is targeting (Optional)
    ///@return response object containing http status code and reply
    response start_resume_playback(int size, char ** uris ,char* device_id = nullptr);
    ///@brief Start or resume playback on provided device
    ///@param device_id Id of the device this command is targeting(Optional)
    ///@return response object containing http status code and reply
    response start_resume_playback(char* device_id = nullptr);
    ///@brief Pause playback on Spotify
    ///@return response object containing http status code and reply
    response pause_playback();
    ///@brief Skip to next track
    ///@return response object containing http status code and reply
    response skip();
    ///@brief Skip to previous track
    ///@return response object containing http status code and reply
    response previous();
    ///@brief get information about the user's available devices
    ///@return response object containing http status code and reply
    response available_devices();
    ///@brief get information about the user's current playback state, including track, track progress, and active device, shuffle etc.
    ///@return response object containing http status code and reply
    response current_playback_state();
    ///@brief Get recently played tracks
    ///@param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    response recently_played_tracks(int limit = 10);
    ///@brief Seek to position of current context
    ///@param time_ms Position in milliseconds to seek to, if the value is greater than the length of the track the player will skip to the next track
    ///@return response object containing http status code and reply
    response seek_to_position(int time_ms);
    ///@brief get users queue, response can be empty or containing episode or track objects
    ///@return response object containing http status code and reply
    response get_queue();
    ///@Brief Set repeat mode, allowed values are REPEAT_OFF, REPEAT_TRACK, REPEAT_CONTEXT
    ///@param mode Repeat mode
    ///@return response object containing http status code and reply
    response repeat_mode(char* mode);
    ///@Brief Set shuffle mode, allowed values are SHUFFLE_ON, SHUFFLE_OFF
    ///@param mode Shuffle mode
    ///@return response object containing http status code and reply
    response shuffle(bool mode);
    ///@Brief Transfer playback to another device
    ///@param device_id Id of the device this command is targeting
    ///@return response object containing http status code and reply
    response transfer_playback(char* device_id);
    ///@Brief Set volume, does not work with all devices(eg. does not work on Phones)
    ///@param value Volume value between 0 and 100
    ///@return response object containing http status code and reply
    response set_volume(int value);
    ///@Brief Add context to queue 
    ///@param context_uri Spotify URI of the context to add to queue
    response add_to_queue(char* context_uri);
    #endif
  #ifdef ENABLE_ALBUM
    //Albums
    ///@brief Get Spotify information for a single album.
    ///@param album_id Spotify ID of the album
    ///@return response object containing http status code and reply
    response get_album(char* album_id);
    ///@brief Get Spotify information for multiple albums identified by their Spotify IDs.
    ///@param size Number of album ids in album_ids array
    ///@param album_ids Array of Spotify IDs of the albums
    ///@return response object containing http status code and reply
    response get_albums(int size,  char** album_ids);
    ///@brief Get Spotify information about an album's tracks. 
    ///@param album_id Spotify ID of the album
    ///@param limit The maximum number of tracks to return. Default: 10. Minimum: 1. Maximum: 50
    ///@param offset The index of the first track to return. Default: 0 (the first object). Use with limit to get the next set of tracks.
    ///@return response object containing http status code and reply
    response get_album_tracks(char* album_id, int limit = 10, int offset = 0);
    ///@brief Get Albums saved to the current user's  music library.
    ///@param limit The maximum number of albums to return. Default: 10. Minimum: 1. Maximum: 50
    ///@param offset The index of the first album to return. Default: 0 (the first object). Use with limit to get the next set of albums.
    ///@return response object containing http status code and reply
    response get_users_saved_albums(int limit = 10, int offset = 0);
    ///@brief Save one or more albums to the current user's  music library.
    ///@param size Number of album ids in album_ids array
    ///@param album_ids Array of Spotify IDs of the albums
    ///@return response object containing http status code and reply
    response save_albums_for_current_user(int size,  char** album_ids);
    ///@brief Remove one or more albums from the current user's  music library.
    ///@param size Number of album ids in album_ids array
    ///@param album_ids Array of Spotify IDs of the albums
    ///@return response object containing http status code and reply
    response remove_users_saved_albums(int size,  char** album_ids);
    ///@brief Check if one or more albums is already saved in the current Spotify user's  music library.
    ///@param size Number of album ids in album_ids array
    ///@param album_ids Array of Spotify IDs of the albums
    ///@return response object containing http status code and reply
    response check_useres_saved_albums(int size,  char** album_ids);
    ///@brief Get a list of new album releases featured in Spotify
    ///@param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    ///@param offset The index of the first item to return. Default: 0 (the first object). Use with limit to get the next set of items.
    ///@param country A country: an ISO 3166-1 alpha-2 country code. Provide this parameter if you want the list of returned items to be relevant to a particular country.
    ///@return response object containing http status code and reply
    response get_new_releases(int limit = 10, int offset = 0, char* country ="");
    #endif
  #ifdef ENABLE_ARTIST
    //Artists
    ///@brief Get Spotify information for a single artist
    ///@param artist_id Spotify ID of the artist
    ///@return response object containing http status code and reply
    response get_artist(char* artist_id);
    ///@brief Get Spotify information for multiple artists 
    ///@param size Number of artist ids in artist_ids array
    ///@param artist_ids Array of Spotify IDs of the artists
    ///@return response object containing http status code and reply
    response get_several_artists(int size,  char** artist_ids);
    ///@brief Get Spotify information about an artist's albums
    ///@param artist_id Spotify ID of the artist
    ///@param size_groups Number of groups in include_groups array
    ///@param include_groups Array of groups to include in the response. Valid values are GROUP_ALBUM, GROUP_SINGLE, GROUP_APPEARS_ON, GROUP_COMPILATION or any combination of these.
    ///@param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    ///@param offset The index of the first album to return. Default: 0 (the first object). Use with limit to get the next set of albums.
    ///@return response object containing http status code and reply
    response get_artist_albums(char* artist_id,int size_groups, char** include_groups, int limit = 10, int offset = 0);
    ///@brief Get Spotify information about an artist's top tracks
    ///@param artist_id Spotify ID of the artist
    ///@param country An ISO 3166-1 alpha-2 country code or the string from_token. Provide this parameter if you want the list of returned items to be relevant to a particular country.
    ///@return response object containing http status code and reply
    response get_artist_top_tracks(char* artist_id, char* country = nullptr);
    ///@brief Get Spotify information about artists related to a single artist
    ///@param artist_id Spotify ID of the artist
    ///@return response object containing http status code and reply
    response get_artist_related_artist(char* artist_id);
    #endif
  #ifdef ENABLE_AUDIOBOOKS
    //Audiobooks (Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    ///@brief Get Spotify information for a single audiobook
    ///@param audiobook_id Spotify ID of the audiobook
    ///@return response object containing http status code and reply
    response get_audiobook(char* audiobook_id);
    ///@brief Get Spotify information for multiple audiobooks
    ///@param size Number of audiobook ids in audiobook_ids array
    ///@param audiobook_ids Array of Spotify IDs of the audiobooks
    ///@return response object containing http status code and reply
    response get_several_audiobooks(int size,  char** audiobook_ids);
    ///@brief Get Spotify information about an audiobook's chapters
    ///@param audiobook_id Spotify ID of the audiobook
    ///@param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    ///@param offset The index of the first chapter to return. Default: 0 (the first object). Use with limit to get the next set of chapters.
    ///@return response object containing http status code and reply
    response get_audiobook_chapters(char* audiobook_id, int limit = 10, int offset = 0);
    /// @brief 
    /// @param limit 
    /// @param offset 
    /// @return 
    response get_users_audiobooks(int limit = 10, int offset = 0);
    /// @brief Save one or more audiobooks to the current user's  music library
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response save_audiobooks_for_current_user(int size,  char** audiobook_ids);
    /// @brief Remove one or more audiobooks from the current user's  music library
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response remove_audiobooks_for_current_user(int size,  char** audiobook_ids);
    /// @brief Check if one or more audiobooks is already saved in the current Spotify user's  music library
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response check_users_saved_audiobooks(int size,  char** audiobook_ids);
    #endif
  #ifdef ENABLE_CATEGORIES
    //Categories
    response get_several_browse_categories(char* country, char* locale, int limit = 10, int offset = 0);
    response get_single_browse_category(char* category_id, char* country, char* locale);
    #endif
  #ifdef ENABLE_CHAPTERS
    //Chapters (Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    response get_chapter(char* chapter_id);
    response get_several_chapters(int size,  char** chapter_ids);
    #endif
  #ifdef ENABLE_EPISODES
    //Episodes
    response get_episode(char* episode_id);
    response get_several_episodes(int size,  char** episode_ids);
    response get_users_saved_episodes(int limit = 10, int offset = 0);
    response save_episodes_for_current_user(int size,  char** episode_ids);
    response remove_episodes_for_current_user(int size,  char** episode_ids);
    response check_users_saved_episodes(int size,  char** episode_ids);
    #endif
  #ifdef ENABLE_GENRES
    //Genres
    response get_available_genre_seeds();
    #endif
  #ifdef ENABLE_MARKETS
    //Markets
    response get_available_markets();
    #endif
  #ifdef ENABLE_PLAYLISTS
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
    #endif
  #ifdef ENABLE_SEARCH
    //Search
    response search(char* q, char* type, int limit = 10, int offset = 0);
    #endif
  #ifdef ENABLE_SHOWS
    //Shows
    response get_show(char* show_id);
    response get_several_shows(int size,  char** show_ids);
    response get_show_episodes(char* show_id, int limit = 10, int offset = 0);
    response get_users_saved_shows(int limit = 10, int offset = 0);
    response save_shows_for_current_user(int size,  char** show_ids);
    response remove_shows_for_current_user(int size,  char** show_ids);
    response check_users_saved_shows(int size,  char** show_ids);
    #endif
  #ifdef ENABLE_TRACKS
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
    #endif
  #ifdef ENABLE_USERS    
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
    #endif
  #ifdef ENABLE_SIMPIFIED
    //Simplified versions of the above
    String current_track_name();
    String current_track_id();
    String current_device_id();
    String current_artist_names();
    char* current_device_id(char* device_id);
    char* current_track_id(char* track_id);
    char* current_track_name(char* track_name);
    char* current_artist_names(char* artist_names);
    bool is_playing();
    #endif
    //String convert_id_to_uri(String id, String type);
    char convert_id_to_uri(char* id, char* type);
    char* convert_id_to_uri(char* id, char* type, char* uri); 
    

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

    char* array_to_char(int size,  char** array, char* result);//Convert array of chars to one comma separated char
    void array_to_json_array(int size,  char** array, char* data, int data_size = _max_char_size);//Convert array of chars to one json array
  #ifdef ENABLE_TRACKS
    bool is_valid_value(float param);//Check if recommendation value is valid
    bool is_valid_value(int param);//Check if recommendation value is valid
    void populate_char_values(std::map<char*, char*>& map, recommendations& recom);//Populate recommendation char values
    void populate_float_values(std::map<char*, float>& map, recommendations& recom);//Populate recommendation float values
  #endif
    const char * extract_endpoint(const char* rest_url);//Extract endpoint from url with regex
    
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