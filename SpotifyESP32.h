#ifndef SpotifyESP32
#define SpotifyESP32

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <UrlEncode.h>
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
};
typedef struct{
  int status_code;
  String reply;
} response;

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
    response recently_played_tracks(int limit);
    response seek_to_position(int time_ms);
    response get_queue();
    response repeat_mode(String mode);
    response shuffle(bool mode);
    response transfer_playback(String device_id);
    response set_volume(int value);
    response add_to_queue(String context_uri);
    //Albums
    response get_album(String id);
    response get_albums(String ids);
    response get_album_tracks(String id, int limit, int offset);
    response get_users_saved_albums(int limit, int offset);
    response save_albums_for_current_user(String ids);
    response remove_users_saved_albums(String ids);
    response check_useres_saved_albums(String ids);
    response get_new_releases(String country, int limit, int offset);
    //Artists
    response get_artist(String id);
    response get_several_artists(String ids);
    response get_artist_albums(String id);
    response get_artist_top_tracks(String id);
    response get_artist_related_artist(String id);
    //Audiobooks
    response get_audiobook(String id);
    response get_several_audiobooks(String ids);
    response get_audiobook_chapters(String id);
    response get_users_audiobooks(int limit, int offset);
    response save_audiobooks_for_current_user(String ids);
    response remove_audiobooks_for_current_user(String id);
    response check_users_saved_audiobooks(String ids);
    //Categories
    response get_several_browse_categories(String country, String locale, int limit, int offset);
    response get_single_browse_category(String category_id, String country, String locale);
    //Chapters
    response get_chapter(String id);
    response get_several_chapters(String ids);
    //Episodes
    response get_episode(String id);
    response get_several_episodes(String ids);
    response get_users_saved_episodes(int limit, int offset);
    response save_episodes_for_current_user(String ids);
    response remove_episodes_for_current_user(String id);
    response check_users_saved_episodes(String ids);
    //Genres
    response get_available_genre_seeds();
    //Markets
    response get_available_markets();
    //Playlists
    response get_playlist(String playlist_id, String fields, String additional_type);
    response change_playlist_details(String playlist_id, String name, bool is_public, bool is_collaborative, String description);
    response get_playlist_items(String playlist_id, String fields, String additional_types, int limit, int offset);
    response update_playlist_items(String playlist_id, String uris, int range_start,int insert_before,int range_length, String snapshot_id);
    response add_items_to_playlist(String playlist_id, String uris, int position);
    response remove_playlist_items(String playlist_id, String uris, String snapshot_id);
    response get_current_users_playlists(int limit, int offset);
    response get_user_playlists(String user_id,int limit, int offset);
    response create_playlist(String user_id, String name, bool is_public, bool is_collaborative, String description);
    response get_featured_playlists(String country, String locale, String timestamp,int limit, int offset);
    response get_category_playlists(String category_id, String country, int limit, int offset);
    response get_playlist_cover_image(String playlist_id);
    response add_custom_playlist_cover_image(String playlist_id, String data);
    //Search
    response search(String q, String type, int limit, int offset);
    //Shows
    response get_show(String id);
    response get_several_shows(String ids);
    response get_show_episodes(String id, int limit, int offset);
    response get_users_saved_shows(int limit, int offset);
    response save_shows_for_current_user(String ids);
    response remove_shows_for_current_user(String id);
    response check_users_saved_shows(String ids);
    //Tracks
    response get_track(String track_id);
    response get_several_tracks();
    response get_user_saved_tracks();
    response save_tracks_for_current_user();
    response remove_user_saved_tracks();
    response check_user_saved_tracks();
    response get_tracks_audio_features();
    response get_track_audio_features();
    response get_track_audio_analysis();
    response get_recommendations();
    //Users
    response get_current_user_profile();
    response get_user_top_items();
    response get_user_profile();
    response follow_playlist();
    response unfollow_playlist();
    response get_followed_artists();
    response follow_artists_or_users();
    response unfollow_artists_or_users();
    response check_if_user_follows_artists_or_users();
    response check_if_users_follow_playlist();
    //Simplified versions of the above
    String current_track_name();
    String current_track_id();
    String current_device_id();
    String current_artist_names();
    String convert_id_to_uri(String id, String type);
    String comma_separated_string_to_json(String list);
    bool is_playing();

  private:
    static const int _size_of_possibly_large_char = 600;
    bool get_token();
    void init_response(response* response_obj);
    response RestApiGet(char rest_url[_size_of_possibly_large_char]);
    response RestApiPut(char rest_url[100], String payload = "");
    response RestApiPost(char rest_url[100], String payload = "");
    response RestApiDelete(char rest_url[100], String payload = "");
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