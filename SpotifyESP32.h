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
    //These Are simplified versions of the previous ones
    String current_track_name();
    String current_track_id();
    String current_device_id();
    String current_artist_names();
    String convert_id_to_uri(String id, String type);

  private:
    bool get_token();
    void init_response(response* response_obj);
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