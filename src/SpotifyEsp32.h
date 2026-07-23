#ifndef SpotifyEsp32
#define SpotifyEsp32
//Uncomment the following lines to disable certain features
//#define DISABLE_LIBRARY
//#define DISABLE_PLAYLISTS
//#define DISABLE_METADATA
//#define DISABLE_PLAYER
//#define DISABLE_USER
//#define DISABLE_SIMPLIFIED


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <esp_random.h>
#include "mbedtls/sha256.h"
#include "mbedtls/base64.h"


#ifndef DISABLE_TRACKS
#include <map>
#endif

typedef enum {
  SPOTIFY_LOG_NONE = 0,
  SPOTIFY_LOG_ERROR,
  SPOTIFY_LOG_WARN,
  SPOTIFY_LOG_INFO,
  SPOTIFY_LOG_DEBUG,
  SPOTIFY_LOG_VERBOSE
} spotify_log_level_t;

extern spotify_log_level_t _spotify_log_level;

#define SPOTIFY_LOGV(tag, format, ...) do { if (_spotify_log_level >= SPOTIFY_LOG_VERBOSE) { Serial.printf("[%6lu][V][%s:%d][%s] " format "\n", millis(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } } while (0)
#define SPOTIFY_LOGD(tag, format, ...) do { if (_spotify_log_level >= SPOTIFY_LOG_DEBUG) { Serial.printf("[%6lu][D][%s:%d][%s] " format "\n", millis(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } } while (0)
#define SPOTIFY_LOGI(tag, format, ...) do { if (_spotify_log_level >= SPOTIFY_LOG_INFO) { Serial.printf("[%6lu][I][%s:%d][%s] " format "\n", millis(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } } while (0)
#define SPOTIFY_LOGW(tag, format, ...) do { if (_spotify_log_level >= SPOTIFY_LOG_WARN) { Serial.printf("[%6lu][W][%s:%d][%s] " format "\n", millis(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } } while (0)
#define SPOTIFY_LOGE(tag, format, ...) do { if (_spotify_log_level >= SPOTIFY_LOG_ERROR) { Serial.printf("[%6lu][E][%s:%d][%s] " format "\n", millis(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } } while (0)


namespace spotify_types {
  extern bool SHUFFLE_ON;
  extern bool SHUFFLE_OFF;
  extern const char* REPEAT_OFF;
  extern const char* REPEAT_TRACK;
  extern const char* REPEAT_CONTEXT;
  extern const char* TYPE_ALBUM;
  extern const char* TYPE_ARTIST;
  extern const char* TYPE_TRACK;
  extern const char* TYPE_PLAYLIST;
  extern const char* TYPE_SHOW;
  extern const char* TYPE_EPISODE;
  extern const char* TYPE_AUDIOBOOK;
  extern const char* TOP_TYPE_ARTIST;
  extern const char* TOP_TYPE_TRACKS;
  extern const char* GROUP_ALBUM;
  extern const char* GROUP_SINGLE;
  extern const char* GROUP_APPEARS_ON;
  extern const char* GROUP_COMPILATION;
  extern const char* TIME_RANGE_SHORT;
  extern const char* TIME_RANGE_MEDIUM;
  extern const char* TIME_RANGE_LONG;
  extern const char* FOLLOW_TYPE_ARTIST;
  extern const char* FOLLOW_TYPE_USER;
  constexpr size_t SIZE_OF_ID = 40; 
  constexpr size_t SIZE_OF_URI = 50;
  constexpr size_t SIZE_OF_SECRET_ID = 100;
  constexpr size_t SIZE_OF_REFRESH_TOKEN = 300;
}

/// @brief Response object containing http status code and reply
typedef struct{
  int status_code;
  JsonDocument reply;
} response;
/// @brief User token object containing user id, secret and refresh token
typedef struct{
  const char* client_id;
  const char* client_secret;
  const char* refresh_token;
} user_tokens;
/// @brief HTTPS header object
typedef struct{
  int http_code;
  size_t content_length;
  String content_type;
  String error;
} header_resp;

typedef void (*AsyncCallback)(response);


/// @brief Print response object
/// @param response_obj Response object to print
void print_response(response response_obj);

class Spotify {
  public:
  /// @brief Creates a Spotify client without a refresh token.
  /// @param client_id Null-terminated Client ID obtained from the Spotify Developer Dashboard.
  ///        The string must remain valid for the lifetime of the Spotify object.
  ///        If credentials are set later at runtime, provide a writable buffer with sufficient size.
  /// @param client_secret Null-terminated Client Secret obtained from the Spotify Developer Dashboard.
  ///        The string must remain valid for the lifetime of the Spotify object.
  ///        If credentials are set later at runtime, provide a writable buffer with sufficient size.
  /// @param max_num_retry Maximum number of automatic retries per HTTP request (default: 3).
  Spotify(const char* client_id, const char* client_secret, int max_num_retry = 3);

  /// @brief Creates a Spotify client with an existing refresh token.
  /// @param client_id Null-terminated Client ID obtained from the Spotify Developer Dashboard.
  ///        The string must remain valid for the lifetime of the Spotify object.
  /// @param client_secret Null-terminated Client Secret obtained from the Spotify Developer Dashboard.
  ///        The string must remain valid for the lifetime of the Spotify object.
  /// @param refresh_token Null-terminated refresh token previously obtained via OAuth.
  ///        The string must remain valid for the lifetime of the Spotify object.
  ///        If set later at runtime, provide a writable buffer with sufficient size.
  /// @param max_num_retry Maximum number of automatic retries per HTTP request (default: 3).
  Spotify(const char* client_id, const char* client_secret, const char* refresh_token, int max_num_retry = 3);

  /// @brief Destructor. Releases allocated resources and stops internal services if running.
  ~Spotify();

  /// @brief Starts the local authentication server and initializes the OAuth flow.
  ///        Must be called before attempting authenticated API requests.
  void begin();

  /// @brief Handles incoming HTTP client requests required for OAuth authentication.
  ///        Should be called regularly inside the main loop.
  void handle_client();

  /// @brief Checks whether a valid access token is available.
  /// @return true if authentication is complete and a valid access token is present.
  bool is_auth();

  #ifndef DISABLE_LIBRARY
    /// @brief Save one or more items (tracks, albums, episodes, shows, audiobooks, artists, users, playlists) to the current user’s library.
    /// @param size Number of URIs in the uris array.
    /// @param uris Array of Spotify URIs (max 40).
    /// @return response object containing http status code and reply.
    response save_items_to_library(int size, const char** uris);

    /// @brief Remove one or more items from the current user’s library.
    /// @param size Number of URIs in the uris array.
    /// @param uris Array of Spotify URIs (max 40).
    /// @return response object containing http status code and reply.
    response remove_items_from_library(int size, const char** uris);

    /// @brief Check if one or more items are already saved in the current user’s library.
    /// @param size Number of URIs in the uris array.
    /// @param uris Array of Spotify URIs (max 40).
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply
    response check_users_saved_items(int size, const char** uris, JsonDocument filter = JsonDocument());

    /// @brief Get a list of the albums saved in the current user’s library.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_users_saved_albums(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get a list of the audiobooks saved in the current user’s library.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_users_saved_audiobooks(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get a list of the episodes saved in the current user’s library.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_users_saved_episodes(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get a list of the shows saved in the current user’s library.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_users_saved_shows(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get a list of the tracks saved in the current user’s library.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_users_saved_tracks(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get the list of artists followed by the current user.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param after The cursor to use for pagination (Optional).
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_followed_artists(int limit = 10, const char* after = nullptr, JsonDocument filter = JsonDocument());
  #endif

  #ifndef DISABLE_PLAYLISTS
    /// @brief Change a playlist’s name, description, or public/collaborative state.
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param details JsonDocument with the fields to update (name, public, collaborative, description).
    /// @return response object containing http status code and reply.
    response change_playlist_details(const char* playlist_id, JsonDocument details);

    /// @brief Create a new playlist for the current user.
    /// @param name The name of the playlist.
    /// @param description Description (Optional).
    /// @param is_public Whether the playlist is public (default true).
    /// @param collaborative Whether the playlist is collaborative (default false).
    /// @return response object containing http status code and reply.
    response create_playlist(const char* name, const char* description = nullptr, bool is_public = true, bool collaborative = false);

    /// @brief Get a list of the current user’s playlists.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_current_users_playlists(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get full details of a playlist.
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_playlist(const char* playlist_id, JsonDocument filter = JsonDocument());

    /// @brief Get the cover image(s) of a playlist.
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_playlist_cover_image(const char* playlist_id, JsonDocument filter = JsonDocument());

    /// @brief Upload a custom cover image for a playlist.
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param image_data Base64-encoded JPEG image data.
    /// @return response object containing http status code and reply.
    response upload_custom_playlist_cover_image(const char* playlist_id, const char* image_data);

    /// @brief Add one or more items to a playlist.
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param size Number of URIs in the uris array.
    /// @param uris Array of Spotify URIs (track or episode).
    /// @param position The position to insert the items (0-based, -1 = end).
    /// @return response object containing http status code and reply.
    response add_items_to_playlist(const char* playlist_id, int size, const char** uris, int position = -1);

    /// @brief Remove one or more items from a playlist.
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param size Number of URIs in the uris array.
    /// @param uris Array of Spotify URIs to remove.
    /// @param snapshot_id The playlist snapshot ID (Optional).
    /// @return response object containing http status code and reply.
    response remove_playlist_items(const char* playlist_id, int size, const char** uris, const char* snapshot_id = nullptr);

    /// @brief Reorder or replace items in a playlist.
    /// @param playlist_id The Spotify ID of the playlist
    /// @param size The size of the URI array
    /// @param uris Array of Spotify URIs (for replace) or empty for reorder only.
    /// @param range_start Start position of the range to move (for reorder).
    /// @param insert_before Position to insert the range before.
    /// @param range_length Length of the range to move (for reorder).
    /// @param snapshot_id The playlist snapshot ID (Optional).
    /// @return response object containing http status code and reply.
    response update_playlist_items(const char* playlist_id, int size = 0, const char** uris = nullptr, int range_start = 0, int insert_before = 0, int range_length = 0, const char* snapshot_id = nullptr);

    /// @brief Get full details of the items of a playlist (new endpoint replacing old /tracks).
    /// @param playlist_id The Spotify ID of the playlist.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_playlist_items(const char* playlist_id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
  #endif

  #ifndef DISABLE_METADATA
    /// @brief Get Spotify catalog information for a single album.
    /// @param id The Spotify ID of the album.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_album(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information about an album’s tracks.
    /// @param id The Spotify ID of the album.
    /// @param limit The maximum number of tracks to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first track to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_album_tracks(const char* id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information for a single artist.
    /// @param id The Spotify ID of the artist.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_artist(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information about an artist’s albums.
    /// @param id The Spotify ID of the artist.
    /// @param limit The maximum number of albums to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first album to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided)..
    /// @return response object containing http status code and reply.
    response get_artists_albums(const char* id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information for a single audiobook.
    /// @param id The Spotify ID of the audiobook.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_audiobook(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information about an audiobook’s chapters.
    /// @param id The Spotify ID of the audiobook.
    /// @param limit The maximum number of chapters to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first chapter to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_audiobook_chapters(const char* id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information for a single audiobook chapter.
    /// @param id The Spotify ID of the chapter.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_chapter(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information for a single episode.
    /// @param id The Spotify ID of the episode.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_episode(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information for a single show.
    /// @param id The Spotify ID of the show.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_show(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information about a show’s episodes.
    /// @param id The Spotify ID of the show.
    /// @param limit The maximum number of episodes to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param offset The index of the first episode to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_show_episodes(const char* id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());

    /// @brief Get Spotify catalog information for a single track.
    /// @param id The Spotify ID of the track.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_track(const char* id, JsonDocument filter = JsonDocument());

    /// @brief Search for albums, artists, playlists, tracks, shows, episodes or audiobooks.
    /// @param q The search query.
    /// @param type Comma-separated list of item types (album,artist,playlist,track,show,episode,audiobook).
    /// @param limit The maximum number of items to return. Default: 5. Minimum: 1. Maximum: 10.
    /// @param offset The index of the first item to return. Default: 0.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response search_for_item(const char* q, const char* type, int limit = 5, int offset = 0, JsonDocument filter = JsonDocument());
  #endif

  #ifndef DISABLE_USER
  /// @brief Get detailed profile information about the current user.
  /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
  /// @return response object containing http status code and reply.
  response get_current_users_profile(JsonDocument filter = JsonDocument());

  /// @brief Get the current user’s top artists or tracks.
  /// @param type The type of entity to return ("artists" or "tracks").
  /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
  /// @param offset The index of the first item to return. Default: 0.
  /// @param time_range Over what time frame the affinities are computed ("short_term", "medium_term", "long_term").
  /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
  /// @return response object containing http status code and reply.
  response get_users_top_items(const char* type, int limit = 10, int offset = 0, const char* time_range = nullptr, JsonDocument filter = JsonDocument());

  #endif

  #ifndef DISABLE_PLAYER
    /// @brief Add an item to the end of the user’s playback queue.
    /// @param uri Spotify URI of the item to add.
    /// @param device_id Id of the device this command is targeting (Optional).
    /// @return response object containing http status code and reply.
    response add_item_to_queue(const char* uri, const char* device_id = nullptr);

    /// @brief Get information about a user’s available devices.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_available_devices(JsonDocument filter = JsonDocument());

    /// @brief Get the object currently being played on the user’s account.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_currently_playing_track(JsonDocument filter = JsonDocument());

    /// @brief Get information about the user’s current playback state.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_playback_state(JsonDocument filter = JsonDocument());

    /// @brief Get tracks from the current user’s recently played tracks.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_recently_played_tracks(int limit = 10, JsonDocument filter = JsonDocument());

    /// @brief Get the list of items in the user’s playback queue.
    /// @param filter JsonDocument containing the fields to filter (Optional, returns all fields if not provided).
    /// @return response object containing http status code and reply.
    response get_users_queue(JsonDocument filter = JsonDocument());

    /// @brief Pause playback on the user’s account.
    /// @return response object containing http status code and reply.
    response pause_playback();

    /// @brief Seek to the given position in the user’s currently playing track.
    /// @param position_ms Position in milliseconds to seek to.
    /// @return response object containing http status code and reply.
    response seek_to_position(int position_ms);

    /// @brief Set the repeat mode for the user’s playback.
    /// @param state Repeat mode ("off", "track", "context").
    /// @return response object containing http status code and reply.
    response set_repeat_mode(const char* state);

    /// @brief Set the volume for the user’s playback.
    /// @param volume_percent Volume value between 0 and 100.
    /// @return response object containing http status code and reply.
    response set_volume(int volume_percent);

    /// @brief Skip to the next track in the user’s queue.
    /// @return response object containing http status code and reply.
    response skip_to_next();

    /// @brief Skip to the previous track in the user’s queue.
    /// @return response object containing http status code and reply.
    response skip_to_previous();

    /// @brief Start a new playback or resume current playback.
    /// @param device_id Id of the device this command is targeting (Optional).
    /// @return response object containing http status code and reply.
    response start_a_users_playback(const char* device_id = nullptr);

    /// @brief Toggle shuffle on or off for the user’s playback.
    /// @param state Shuffle mode (true = on, false = off).
    /// @return response object containing http status code and reply.
    response toggle_shuffle(bool state);

    /// @brief Transfer playback to a new device and optionally begin playback.
    /// @param device_id Id of the device this command is targeting.
    /// @return response object containing http status code and reply.
    response transfer_playback(const char* device_id);
  #endif

  #ifndef DISABLE_SIMPLIFIED
    /// @brief Returns the name of the currently playing track.
    /// @return Track name as String. Empty if no track is active.
    String current_track_name();

    /// @brief Returns the Spotify ID of the currently playing track.
    /// @return Track ID as String. Empty if unavailable.
    String current_track_id();

    /// @brief Returns the Spotify ID of the currently active device.
    /// @return Device ID as String. Empty if no active device.
    String current_device_id();

    /// @brief Returns a comma-separated list of artist names
    ///        for the currently playing track.
    /// @return Artist names as String. Empty if unavailable.
    String current_artist_names();

    /// @brief Writes the current device ID into a provided buffer.
    /// @param device_id Pre-allocated buffer to store the device ID.
    /// @return Pointer to the provided buffer.
    char* current_device_id(char* device_id);

    /// @brief Writes the current track ID into a provided buffer.
    /// @param track_id Pre-allocated buffer to store the track ID.
    /// @return Pointer to the provided buffer.
    char* current_track_id(char* track_id);

    /// @brief Writes the current track name into a provided buffer.
    /// @param track_name Pre-allocated buffer to store the track name.
    /// @return Pointer to the provided buffer.
    char* current_track_name(char* track_name);

    /// @brief Writes the current artist names into a provided buffer.
    /// @param artist_names Pre-allocated buffer to store
    ///        a comma-separated list of artist names.
    /// @return Pointer to the provided buffer.
    char* current_artist_names(char* artist_names);

    /// @brief Indicates whether playback is currently active.
    /// @return True if the active device is playing.
    bool is_playing();

    /// @brief Indicates whether the current device supports volume control.
    /// @return True if volume can be modified on the active device.
    bool volume_modifyable();

    /// @brief Returns the album cover image URL of the currently playing track.
    /// @param image_size_idx Index of the image in the Spotify image array
    ///        (typically 0 = largest).
    /// @return Image URL as String. Empty if unavailable.
    String get_current_album_image_url(int image_size_idx);
  #endif

    /// @brief Converts a Spotify ID to a Spotify URI.
    /// @param id Null-terminated Spotify object ID.
    /// @param type Null-terminated object type string.
    /// @param uri Pre-allocated char buffer where the resulting URI will be written.
    ///            The buffer must be large enough to hold the full URI including the null terminator.
    /// @return Pointer to the provided URI buffer.
    char* convert_id_to_uri(const char* id, const char* type, char* uri);

    /// @brief Returns the stored user credential data.
    /// @return A user_tokens struct containing client ID, client secret, and refresh token.
    user_tokens get_user_tokens();

    /// @brief Checks whether a valid access token is currently stored.
    /// @return True if an access token is available.
    bool has_access_token();

    /// @brief Requests a new access token using the stored refresh token.
    /// @return True if the token request was successful.
    bool get_access_token();

    /// @brief Exchanges an authorization code for a refresh token.
    /// @param auth_code Null-terminated authorization code received from Spotify OAuth redirect.
    /// @param redirect_uri Null-terminated redirect URI used during the authorization request.
    ///        Must match the URI configured in the Spotify Developer Dashboard.
    /// @return True if the token exchange was successful.
    bool get_refresh_token(const char* auth_code, const char* redirect_uri);

    /// @brief Sets the OAuth scopes for the authorization request.
    /// @param scopes Null-terminated string containing space-separated scope names.
    void set_scopes(const char* scopes);

    /// @brief Sets the internal logging level.
    /// @param spotify_log_level Logging level enum value.
    void set_log_level(spotify_log_level_t spotify_log_level);

   /// @brief Execute a Spotify API call asynchronously.
   /// @param fn A lambda function wrapping the Spotify member function to call. Any required arguments must be bound inside the lambda.
   /// @param callback Function pointer called once the async function completes. Receives the `response` object.
    void async(std::function<response()> fn, AsyncCallback callback);

    /// @brief Stops internal services and releases allocated resources.
    /// After calling this function, the object should not be used anymore unless reinitialized.
    void end();
    
  private:
    /// @brief Logging tag used for debug output.
    static constexpr const char* _TAG = "Spotify";

    /// @brief OAuth redirect URI registered in the Spotify Developer Dashboard.
    const char* _redirect_uri = "https://spotifyesp32.vercel.app/api/spotify/callback";

    /// @brief OAuth authorization host.
    const char* _auth_host = "spotifyesp32.vercel.app";

    /// @brief Spotify Web API host.
    const char* _host = "api.spotify.com";

    /// @brief Spotify Accounts service host (token endpoint).
    const char* _token_host = "accounts.spotify.com";

    /// @brief Base URL for Spotify Web API endpoints.
    const char* _base_url = "https://api.spotify.com/v1/";

    /// @brief Maximum number of items allowed per API batch request.
    static const int _max_num_items = 20;

    /// @brief Maximum size for URI aggregation buffers.
    /// 35 = approx. URI length, + comma, + buffer for URL overhead.
    static const int _max_char_size = 35 * _max_num_items + 150;

    /// @brief Default buffer size for standard URLs.
    const int _standard_url_size = 100;

    /// @brief Maximum length of a Spotify URI including null terminator.
    static const int _size_of_uri = 45;

    /// @brief Maximum length of a Spotify ID including null terminator.
    static const int _size_of_id = 25;

    /// @brief OAuth state string length (without null terminator).
    static const int _state_len = 25;

    /// @brief Random OAuth state used for CSRF protection.
    char _random_state[_state_len + 1];

    /// @brief True if credentials are provided at runtime instead of constructor.
    bool _no_credentials = false;

    /// @brief Maximum number of automatic retries per request.
    int _max_num_retry = 3;

    /// @brief Current retry counter.
    int _retry;

    /// @brief Request timeout in milliseconds.
    int _timeout = 5000;

    /// @brief User refresh token.
    char _refresh_token[300] = "";

    /// @brief Authorization code received during OAuth flow.
    char _auth_code[1024] = "";

    /// @brief Client ID provided by user.
    char _client_id[100] = "";

    /// @brief Client secret provided by user.
    char _client_secret[100] = "";

    /// @brief Current access token used for API requests.
    char _access_token[400];

    /// @brief Char to hold potential custom scopes.
    const char* _custom_scopes;

    /// @brief Secure HTTPS client used for all API communication.
    WiFiClientSecure _client;

    /// @brief Root certificate authority for Spotify endpoints (stored in flash).
    const char* _spotify_root_ca PROGMEM = \
    "-----BEGIN CERTIFICATE-----\n"\
    "MIIEyDCCA7CgAwIBAgIQDPW9BitWAvR6uFAsI8zwZjANBgkqhkiG9w0BAQsFADBh\n"\
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"\
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"\
    "MjAeFw0yMTAzMzAwMDAwMDBaFw0zMTAzMjkyMzU5NTlaMFkxCzAJBgNVBAYTAlVT\n"\
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxMzAxBgNVBAMTKkRpZ2lDZXJ0IEdsb2Jh\n"\
    "bCBHMiBUTFMgUlNBIFNIQTI1NiAyMDIwIENBMTCCASIwDQYJKoZIhvcNAQEBBQAD\n"\
    "ggEPADCCAQoCggEBAMz3EGJPprtjb+2QUlbFbSd7ehJWivH0+dbn4Y+9lavyYEEV\n"\
    "cNsSAPonCrVXOFt9slGTcZUOakGUWzUb+nv6u8W+JDD+Vu/E832X4xT1FE3LpxDy\n"\
    "FuqrIvAxIhFhaZAmunjZlx/jfWardUSVc8is/+9dCopZQ+GssjoP80j812s3wWPc\n"\
    "3kbW20X+fSP9kOhRBx5Ro1/tSUZUfyyIxfQTnJcVPAPooTncaQwywa8WV0yUR0J8\n"\
    "osicfebUTVSvQpmowQTCd5zWSOTOEeAqgJnwQ3DPP3Zr0UxJqyRewg2C/Uaoq2yT\n"\
    "zGJSQnWS+Jr6Xl6ysGHlHx+5fwmY6D36g39HaaECAwEAAaOCAYIwggF+MBIGA1Ud\n"\
    "EwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFHSFgMBmx9833s+9KTeqAx2+7c0XMB8G\n"\
    "A1UdIwQYMBaAFE4iVCAYlebjbuYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAd\n"\
    "BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQG\n"\
    "CCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKG\n"\
    "NGh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RH\n"\
    "Mi5jcnQwQgYDVR0fBDswOTA3oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29t\n"\
    "L0RpZ2lDZXJ0R2xvYmFsUm9vdEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwC\n"\
    "ATAHBgVngQwBATAIBgZngQwBAgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG\n"\
    "9w0BAQsFAAOCAQEAkPFwyyiXaZd8dP3A+iZ7U6utzWX9upwGnIrXWkOH7U1MVl+t\n"\
    "wcW1BSAuWdH/SvWgKtiwla3JLko716f2b4gp/DA/JIS7w7d7kwcsr4drdjPtAFVS\n"\
    "slme5LnQ89/nD/7d+MS5EHKBCQRfz5eeLjJ1js+aWNJXMX43AYGyZm0pGrFmCW3R\n"\
    "bpD0ufovARTFXFZkAdl9h6g4U5+LXUZtXMYnhIHUfoyMo5tS58aI7Dd8KvvwVVo4\n"\
    "chDYABPPTHPbqjc1qCmBaZx2vN4Ye5DUys/vZwP9BFohFrH/6j/f3IL16/RZkiMN\n"\
    "JCqVJUzKoZHm1Lesh3Sz8W2jmdv51b2EQJ8HmA==\n"\
    "-----END CERTIFICATE-----\n";

    /// @brief Generates a random OAuth state string.
    /// @param state Buffer to store generated state.
    /// @param len Length of buffer (including null terminator).
    void get_random_state(char* state, size_t len);

    /// @brief URL-encodes a string.
    /// @param src Null-terminated input string.
    /// @param dest Destination buffer.
    /// @param destSize Size of destination buffer.
    void urlEncode(const char* src, char* dest, size_t destSize);

    /// @brief Extracts API endpoint path from a full REST URL.
    /// @param rest_url Full REST URL.
    /// @return Endpoint path as String.
    String extract_endpoint(const char* rest_url);

    /// @brief Retrieves a new access token using the stored refresh token.
    /// @return True if token retrieval was successful.
    bool get_token();

    /// @brief Sends a token request to the Spotify Accounts service.
    /// @param payload URL-encoded request payload.
    /// @param payload_len Length of payload in bytes.
    /// @return True if HTTP request succeeded.
    bool token_base_req(const char* payload, size_t payload_len);

    /// @brief Validates HTTP response code.
    /// @param code HTTP status code.
    /// @return True if status code indicates success.
    bool valid_http_code(int code);

    /// @brief Processes HTTP response headers.
    /// @return Parsed header response struct.
    header_resp process_headers();

    /// @brief Processes HTTP response body.
    /// @param header Parsed header information.
    /// @param filter Optional ArduinoJson filter document.
    /// @return Parsed JsonDocument.
    JsonDocument process_response(header_resp header, JsonDocument filter = JsonDocument());

    /// @brief Initializes a response structure.
    /// @param response_obj Pointer to response struct.
    void init_response(response* response_obj);

    /// @brief Performs a generic REST API request.
    /// @param rest_url Full API URL.
    /// @param type HTTP method (GET, POST, PUT, DELETE).
    /// @param payload_size Size of payload in bytes.
    /// @param payload Optional request body.
    /// @param filter Optional ArduinoJson filter.
    /// @param content_type Content-Type header (default: application/json).
    /// @return Response struct containing HTTP status and payload.
    response RestApi(const char* rest_url, const char* type, int payload_size = 0, const char* payload = nullptr, JsonDocument filter = JsonDocument(), const char* content_type = "application/json");

    /// @brief Performs a PUT request.
    response RestApiPut(const char* rest_url, int payload_size = 0,const char* payload = nullptr, const char* content_type = "application/json"); 

    /// @brief Performs a POST request.
    response RestApiPost(const char* rest_url, int payload_size = 0, const char* payload = nullptr);

    /// @brief Performs a DELETE request.
    response RestApiDelete(const char* rest_url, int payload_size = 0, const char* payload = nullptr);

    /// @brief Performs a GET request.
    response RestApiGet(const char* rest_url, JsonDocument filter = JsonDocument());  
};
#endif