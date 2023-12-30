#ifndef SpotifyESP32
#define SpotifyESP32

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <UrlEncode.h>
#include <map>
namespace Spotify_types{
  extern bool SHUFFLE_ON;
  extern bool SHUFFLE_OFF;
  extern String REPEAT_OFF;
  extern String REPEAT_TRACK;
  extern String REPEAT_CONTEXT;
  extern String TYPE_ALBUM;
  extern String TYPE_ARTIST;
  extern String TYPE_TRACK;
  extern String TYPE_PLAYLIST;
  extern String GROUP_ALBUM;
  extern String GROUP_SINGLE;
  extern String GROUP_APPEARS_ON;
  extern String GROUP_COMPILATION;
  extern String TIME_RANGE_SHORT;
  extern String TIME_RANGE_MEDIUM;
  extern String TIME_RANGE_LONG;
  extern String TOP_TYPE_ARTIST;
  extern String TOP_TYPE_TRACKS;
};
typedef struct{
  int status_code;
  String reply;
} response;

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
};


void print_response(response response_obj);

class Spotify {
  public:
    Spotify(char* refresh_token, char* redirect_uri, char* client_id, char* client_secret, bool debug_on);
    //Player
    response currently_playing();
    response play_uri(String track_uri);
    response start_playback();
    response pause_playback();
    response skip();
    response previous();
    response available_devices();
    response current_playback_state();
    response recently_played_tracks(int limit = 10);
    response seek_to_position(int time_ms);
    response get_queue();
    response repeat_mode(String mode);
    response shuffle(bool mode);
    response transfer_playback(String device_id);
    response set_volume(int value);
    response add_to_queue(String context_uri);
    //Albums
    response get_album(String album_id);
    response get_albums(int size,  char** album_ids);
    response get_album_tracks(String album_id, int limit = 10, int offset = 0);
    response get_users_saved_albums(int limit = 10, int offset = 0);
    response save_albums_for_current_user(int size,  char** album_ids);
    response remove_users_saved_albums(int size,  char** album_ids);
    response check_useres_saved_albums(int size,  char** album_ids);
    response get_new_releases(String country, int limit = 10, int offset = 0);
    //Artists
    response get_artist(String artist_id);
    response get_several_artists(int size,  char** artist_ids);
    response get_artist_albums(String artist_id, String include_groups, int limit = 10, int offset = 0);
    response get_artist_top_tracks(String artist_id, String country);
    response get_artist_related_artist(String artist_id);
    //Audiobooks (Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    response get_audiobook(String audiobook_id);
    response get_several_audiobooks(int size,  char** audiobook_ids);
    response get_audiobook_chapters(String audiobook_id, int limit = 10, int offset = 0);
    response get_users_audiobooks(int limit = 10, int offset = 0);
    response save_audiobooks_for_current_user(int size,  char** audiobook_ids);
    response remove_audiobooks_for_current_user(int size,  char** audiobook_ids);
    response check_users_saved_audiobooks(int size,  char** audiobook_ids);
    //Categories
    response get_several_browse_categories(String country, String locale, int limit = 10, int offset = 0);
    response get_single_browse_category(String category_id, String country, String locale);
    //Chapters (Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    response get_chapter(String chapter_id);
    response get_several_chapters(int size,  char** chapter_ids);
    //Episodes
    response get_episode(String episode_id);
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
    response get_playlist(String playlist_id, String fields);
    response change_playlist_details(String playlist_id, String name, bool is_public, bool is_collaborative, String description);
    response get_playlist_items(String playlist_id, String fields, int limit = 10, int offset = 0);
    response update_playlist_items(String playlist_id, int size,  char** uris = nullptr, int range_length = 1, int range_start = 0, int insert_before = 1 );
    response add_items_to_playlist(String playlist_id, int size,  char** uris, int position = 0);
    response remove_playlist_items(String playlist_id, int size,  char** uris);
    response get_current_users_playlists(int limit = 10, int offset = 0);
    response get_user_playlists(String user_id,int limit = 10, int offset = 0);
    response create_playlist(String user_id, String name, bool is_public, bool is_collaborative, String description);
    response get_featured_playlists(String country, String locale, String timestamp,int limit = 10, int offset = 0);
    response get_category_playlists(String category_id, String country, int limit = 10, int offset = 0);
    response get_playlist_cover_image(String playlist_id);
    response add_custom_playlist_cover_image(String playlist_id, String data);
    //Search
    response search(String q, String type, int limit = 10, int offset = 0);
    //Shows
    response get_show(String show_id);
    response get_several_shows(int size,  char** show_ids);
    response get_show_episodes(String show_id, int limit = 10, int offset = 0);
    response get_users_saved_shows(int limit = 10, int offset = 0);
    response save_shows_for_current_user(int size,  char** show_ids);
    response remove_shows_for_current_user(int size,  char** show_ids);
    response check_users_saved_shows(int size,  char** show_ids);
    //Tracks
    response get_track(String track_id);
    response get_several_tracks(int size,  char** track_ids);
    response get_user_saved_tracks(int limit = 10, int offset = 0);
    response save_tracks_for_current_user(int size,  char** track_ids);
    response remove_user_saved_tracks(int size,  char** track_ids);
    response check_user_saved_tracks(int size,  char** track_ids);
    response get_tracks_audio_features(int size,  char** track_ids);
    response get_track_audio_features(String track_id);
    response get_track_audio_analysis(String track_id);
    response get_recommendations(recommendations& recom, int limit = 10);
    
    //Users
    response get_current_user_profile();
    response get_user_top_items(String type, String time_range = "medium_term", int limit = 10, int offset = 0);
    response get_user_profile(String user_id);
    response follow_playlist(String playlist_id, bool is_public);
    response unfollow_playlist(String playlist_id);
    response get_followed_artists(String after, String type = "artist", int limit = 10);
    response follow_artists_or_users(String type, int size,  char** artist_user_ids);
    response unfollow_artists_or_users(String type, int size,  char** artist_user_ids);
    response check_if_user_follows_artists_or_users(String type, int size,  char** artist_user_ids);
    response check_if_users_follow_playlist(String playlist_id, int size,  char** user_ids);

    //Simplified versions of the above
    String current_track_name();
    String current_track_id();
    String current_device_id();
    String current_artist_names();
    String convert_id_to_uri(String id, String type);
    bool is_playing();

  private:
    static const int _max_amount_of_items_for_any_endpoint = 20;
    static const int _size_of_possibly_large_char = 35*_max_amount_of_items_for_any_endpoint + 50;// 35 beeing the size of a uri + comma + 50 as buffer
    char string_to_char(String data, int size);
    bool get_token();
    void init_response(response* response_obj);
    response RestApiGet(char rest_url[_size_of_possibly_large_char]);
    response RestApiPut(char rest_url[100], String payload = "");
    response RestApiPost(char rest_url[100], String payload = "");
    response RestApiDelete(char rest_url[100], String payload = "");
    String array_to_string(int size,  char** array);
    String array_to_json_array(int size,  char** array);
    bool is_valid_value(float param);
    bool is_valid_value(int param);
    void populate_char_values(std::map<String, String>& map, recommendations& recom);
    void populate_float_values(std::map<String, float>& map, recommendations& recom);
    char* _refresh_token;
    char* _redirect_uri;
    char* _client_id;
    char* _client_secret;
    int _retry;
    bool _debug_on;
    String _access_token;
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