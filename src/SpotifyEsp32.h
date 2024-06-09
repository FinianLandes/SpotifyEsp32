#ifndef SpotifyEsp32
#define SpotifyEsp32
//Uncomment the following lines to disable certain features
//#define DISABLE_PLAYER
//#define DISABLE_ALBUM
//#define DISABLE_ARTIST
//#define DISABLE_AUDIOBOOKS
//#define DISABLE_CATEGORIES
//#define DISABLE_CHAPTERS
//#define DISABLE_EPISODES
//#define DISABLE_GENRES
//#define DISABLE_MARKETS
//#define DISABLE_PLAYLISTS
//#define DISABLE_SEARCH
//#define DISABLE_SHOWS
//#define DISABLE_TRACKS
//#define DISABLE_USER
//#define DISABLE_SIMPLIFIED
//#define DISABLE_WEB_SERVER


#include <WiFiClientSecure.h>


#include <ArduinoJson.h>
#include <base64.h>

#ifndef DISABLE_TRACKS
#include <map>
#endif
#ifndef DISABLE_WEB_SERVER
#include <WebServer.h>
#endif



namespace Spotify_types {
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
typedef struct{
  const char* client_id;
  const char* client_secret;
  const char* refresh_token;
} user_tokens;
typedef struct{
  int http_code;
  size_t content_length;
  String content_type;
  String error;
} header_resp;
/// @brief Recommendation object, used to create recommendations
struct recommendations {
  const char** seed_artists;
  int seed_artists_size = 0;
  const char** seed_genres;
  int seed_genres_size = 0;
  const char** seed_tracks;
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

/// @brief Print response object
/// @param response_obj Response object to print
void print_response(response response_obj);
class Spotify {
  public:
    /// @brief Constructor for Spotify object without refresh token
    /// @param client_id Client id from Spotify, if you want to set it during runtime provide an empty char* with enough space 
    /// @param client_secret Client secret from Spotify, if you want to set it during runtime provide an empty char* with enough space 
    /// @param server_port Port for the server to run on(default 80, if 80 is already used use anything above 1024)
    /// @param debug_on Debug mode on or off(default off)
    /// @param max_num_retry Max number of retries for a request(default 3)
    Spotify(const char* client_id, const char* client_secret, int server_port = 80, bool debug_on = false, int max_num_retry = 3);
    /// @brief Constructor for Spotify object with refresh token
    /// @param client_id Client id from Spotify, if you want to set it during runtime provide an empty char* with enough space 
    /// @param client_secret Client secret from Spotify,if you want to set it during runtime provide an empty char* with enough space 
    /// @param refresh_token Refresh token from Spotify, if you want to set it during runtime provide an empty char* with enough space 
    /// @param server_port Port for the server to run on(default 80, if 80 is already used use anything above 1024)
    /// @param debug_on Debug mode on or off(default off)
    /// @param max_num_retry Max number of retries for a request(default 3)
    Spotify(const char* client_id, const char* client_secret, const char* refresh_token, int server_port = 80, bool debug_on = false, int max_num_retry = 3);
    ~Spotify();
    /// @brief start the server and begin authentication
    void begin();
    /// @brief handle client requests necessary for authentication
    void handle_client();
    /// @brief Check if user is authenticated
    /// @return true if user is authenticated
    bool is_auth();
  #ifndef DISABLE_PLAYER
    /// @brief Get information about the user's current playback state, including track, track progress, and active device.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response currently_playing(JsonDocument filter = JsonDocument());
    /// @brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. 
    /// @param context_uri Spotify URI of the context to play (Required)
    /// @param offset Indicates from where in the context playback should start, Only works with albums or Playlists. NEEDS TO BE SET (0) IF ONLY URI IS PROVIDED
    /// @param position_ms Indicates from what position in the context playback should start in milliseconds(Optional)
    /// @param device_id Id of the device this command is targeting (Optional)
    /// @return response object containing http status code and reply
    response start_resume_playback(const char* context_uri, int offset, int position_ms = 0, const char* device_id = nullptr);
    /// @brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. 
    /// @param size Number of uris in uris array
    /// @param uris Array of Spotify URIs of the tracks to play
    /// @param device_id Id of the device this command is targeting (Optional)
    /// @return response object containing http status code and reply
    response start_resume_playback(int size, const char ** uris, const char* device_id = nullptr);
    /// @brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. Targets the currently playing context.
    /// @return response object containing http status code and reply
    response start_resume_playback(const char* device_id = nullptr);
    /// @brief Pause playback on Spotify
    /// @return response object containing http status code and reply
    response pause_playback();
    /// @brief Skip to next track
    /// @return response object containing http status code and reply
    response skip();
    /// @brief Skip to previous track
    /// @return response object containing http status code and reply
    response previous();
    /// @brief get information about the user's available devices
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response available_devices(JsonDocument filter = JsonDocument());
    /// @brief get information about the user's current playback state, including track, track progress, and active device, shuffle etc.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response current_playback_state(JsonDocument filter = JsonDocument());
    /// @brief Get recently played tracks
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response recently_played_tracks(int limit = 10, JsonDocument filter = JsonDocument());
    /// @brief Seek to position of current context
    /// @param time_ms Position in milliseconds to seek to, if the value is greater than the length of the track the player will skip to the next track
    /// @return response object containing http status code and reply
    response seek_to_position(int time_ms);
    /// @brief get users queue, response can be empty or containing episode or track objects
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_queue(JsonDocument filter = JsonDocument());
    /// @brief Set repeat mode, allowed values are REPEAT_OFF, REPEAT_TRACK, REPEAT_CONTEXT
    /// @param mode Repeat mode
    /// @return response object containing http status code and reply
    response repeat_mode(const char* mode);
    /// @brief Set shuffle mode, allowed values are SHUFFLE_ON, SHUFFLE_OFF
    /// @param mode Shuffle mode
    /// @return response object containing http status code and reply
    response shuffle(bool mode);
    /// @brief Transfer playback to another device
    /// @param device_id Id of the device this command is targeting
    /// @return response object containing http status code and reply
    response transfer_playback(const char* device_id);
    /// @brief Set volume, does not work with all devices(eg. does not work on Phones)
    /// @param value Volume value between 0 and 100
    /// @return response object containing http status code and reply
    response set_volume(int value);
    /// @brief Add context to queue 
    /// @param context_uri Spotify URI of the context to add to queue
    response add_to_queue(const char* context_uri);
    #endif
  #ifndef DISABLE_ALBUM
    /// @brief Get Spotify information for a single album.
    /// @param album_id Spotify ID of the album
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_album(const char* album_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple albums identified by their Spotify IDs.
    /// @param size Number of album ids in album_ids array
    /// @param album_ids Array of Spotify IDs of the albums
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_albums(int size,  const char** album_ids, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information about an album's tracks. 
    /// @param album_id Spotify ID of the album
    /// @param limit The maximum number of tracks to return. Default: 10. Minimum: 1. Maximum: 50
    /// @param offset The index of the first track to return. Default: 0 (the first object). Use with limit to get the next set of tracks.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_album_tracks(const char* album_id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Get Albums saved to the current user's  music library.
    /// @param limit The maximum number of albums to return. Default: 10. Minimum: 1. Maximum: 50
    /// @param offset The index of the first album to return. Default: 0 (the first object). Use with limit to get the next set of albums.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_users_saved_albums(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Save one or more albums to the current user's  music library.
    /// @param size Number of album ids in album_ids array
    /// @param album_ids Array of Spotify IDs of the albums
    /// @return response object containing http status code and reply
    response save_albums_for_current_user(int size,  const char** album_ids);
    /// @brief Remove one or more albums from the current user's  music library.
    /// @param size Number of album ids in album_ids array
    /// @param album_ids Array of Spotify IDs of the albums
    /// @return response object containing http status code and reply
    response remove_users_saved_albums(int size,  const char** album_ids);
    /// @brief Check if one or more albums is already saved in the current Spotify user's  music library.
    /// @param size Number of album ids in album_ids array
    /// @param album_ids Array of Spotify IDs of the albums
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_useres_saved_albums(int size, const char** album_ids, JsonDocument filter = JsonDocument());
    /// @brief Get a list of new album releases featured in Spotify
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    /// @param offset The index of the first item to return. Default: 0 (the first object). Use with limit to get the next set of items.
    /// @param country A country: an ISO 3166-1 alpha-2 country code. Provide this parameter if you want the list of returned items to be relevant to a particular country.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_new_releases(int limit = 10, int offset = 0, const char* country = nullptr, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_ARTIST
    /// @brief Get Spotify information for a single artist
    /// @param artist_id Spotify ID of the artist
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_artist(const char* artist_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple artists 
    /// @param size Number of artist ids in artist_ids array
    /// @param artist_ids Array of Spotify IDs of the artists
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_artists(int size,  const char** artist_ids, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information about an artist's albums
    /// @param artist_id Spotify ID of the artist
    /// @param size_groups Number of groups in include_groups array
    /// @param include_groups Array of groups to include in the response. Valid values are GROUP_ALBUM, GROUP_SINGLE, GROUP_APPEARS_ON, GROUP_COMPILATION or any combination of these.
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    /// @param offset The index of the first album to return. Default: 0 (the first object). Use with limit to get the next set of albums.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_artist_albums(const char* artist_id,int size_groups, const char** include_groups, int limit = 10, int offset = 0,JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information about an artist's top tracks
    /// @param artist_id Spotify ID of the artist
    /// @param country An ISO 3166-1 alpha-2 country code or the string from_token. Provide this parameter if you want the list of returned items to be relevant to a particular country.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_artist_top_tracks(const char* artist_id, const char* country = nullptr, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information about artists related to a single artist
    /// @param artist_id Spotify ID of the artist
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_artist_related_artist(const char* artist_id, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_AUDIOBOOKS
    /// @brief Get Spotify information for a single audiobook(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param audiobook_id Spotify ID of the audiobook
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_audiobook(const char* audiobook_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple audiobooks(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_audiobooks(int size, const char** audiobook_ids, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information about an audiobook's chapters(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param audiobook_id Spotify ID of the audiobook
    /// @param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    /// @param offset The index of the first chapter to return. Default: 0 (the first object). Use with limit to get the next set of chapters.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_audiobook_chapters(const char* audiobook_id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Get users saved audiobooks(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_users_audiobooks(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Save one or more audiobooks to the current user's  music library(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response save_audiobooks_for_current_user(int size, const char** audiobook_ids);
    /// @brief Remove one or more audiobooks from the current user's  music library(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response remove_audiobooks_for_current_user(int size,  const char** audiobook_ids);
    /// @brief Check if one or more audiobooks is already saved in the current Spotify user's  music library(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_users_saved_audiobooks(int size,  const char** audiobook_ids, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_CATEGORIES
    /// @brief Get a list of categories used to tag items in Spotify
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param country An ISO 3166-1 alpha-2 country code, if ommited the returned items will not be country-specific
    /// @param locale The desired language, consisting of an ISO 639-1 language code and an ISO 3166-1 alpha-2 country code, joined by an underscore, if ommited the response defaults to American English
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_browse_categories(int limit = 10, int offset = 0, const char* country = nullptr, const char* locale = nullptr, JsonDocument filter = JsonDocument());
    /// @brief Get a single category used to tag items in Spotify
    /// @param category_id Spotify category ID of the category
    /// @param country An ISO 3166-1 alpha-2 country code, if ommited the returned items will not be country-specific
    /// @param locale The desired language, consisting of an ISO 639-1 language code and an ISO 3166-1 alpha-2 country code, joined by an underscore, if ommited the response defaults to American English
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_single_browse_category(const char* category_id, const char* country = nullptr, const char* locale = nullptr, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_CHAPTERS
    /// @brief Get Spotify information for a single chapter, Only Available in US, UK, Canada, Ireland, New Zealand and Australia
    /// @param chapter_id Spotify ID of the chapter
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_chapter(const char* chapter_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple chapters, Only Available in US, UK, Canada, Ireland, New Zealand and Australia
    /// @param size Number of chapter ids in chapter_ids array
    /// @param chapter_ids Array of Spotify IDs of the chapters
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_chapters(int size,  const char** chapter_ids, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_EPISODES
    /// @brief Get Spotify information for a single episode
    /// @param episode_id Spotify ID of the episode
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_episode(const char* episode_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple episodes
    /// @param size Number of episode ids in episode_ids array
    /// @param episode_ids Array of Spotify IDs of the episodes
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_episodes(int size,  const char** episode_ids, JsonDocument filter = JsonDocument());
    /// @brief Get users saved episodes
    /// @param limit The maximum number of items to return,
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_users_saved_episodes(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Save one or more episodes to the current user's  music library
    /// @param size Number of episode ids in episode_ids array
    /// @param episode_ids Array of Spotify IDs of the episodes
    /// @return response object containing http status code and reply
    response save_episodes_for_current_user(int size,  const char** episode_ids);
    /// @brief Remove one or more episodes from the current user's  music library
    /// @param size Number of episode ids in episode_ids array
    /// @param episode_ids Array of Spotify IDs of the episodes
    /// @return response object containing http status code and reply
    response remove_episodes_for_current_user(int size,  const char** episode_ids);
    /// @brief Check if one or more episodes is already saved in the current Spotify user's  music library
    /// @param size Number of episode ids in episode_ids array
    /// @param episode_ids Array of Spotify IDs of the episodes
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_users_saved_episodes(int size,  const char** episode_ids, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_GENRES
    /// @brief Get a list of available genre seeds for recommendations
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_available_genre_seeds(JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_MARKETS
    /// @brief Get a list of available markets for recommendations
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_available_markets(JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_PLAYLISTS
    /// @brief Get Spotify information for a single playlist
    /// @param playlist_id Spotify ID of the playlist
    /// @param size Number of fields in fields array
    /// @param fields Array of fields to return, leave empty to return all fields
    /// @param additional_types A comma-separated list of item types that your client supports besides the default track type. Valid types are: track and episode.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_playlist(const char* playlist_id, int size = 0, const char** fields = nullptr, int size_of_additional_types = 0, const char ** additional_types = nullptr, JsonDocument filter = JsonDocument());
    /// @brief Change details of a playlist
    /// @param playlist_id ID of the playlist
    /// @param name Set the name of the playlist
    /// @param is_public Set the playlist to public or not
    /// @param is_collaborative Set the playlist to collaborative or not
    /// @param description Set the description of the playlist
    /// @return response object containing http status code and reply
    response change_playlist_details(const char* playlist_id, const char* name, bool is_public, bool is_collaborative, const char* description);
    /// @brief Get Items of a playlist
    /// @param playlist_id Id of the playlist
    /// @param fields Filters for the query: a comma-separated list of the fields to return. If omitted, all fields are returned. For example, to get just the total number of items and the request limit: 
    /// @param limit Number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_playlist_items(const char* playlist_id, const char* fields, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Either reorder or replace items in a playlist depending on the request's parameters
    /// @param playlist_id Id of the playlist
    /// @param size Size of uris array
    /// @param uris Array of Spotify URIs this will overwrite all existing items in the playlist, If items should only be reordered pass nullptr
    /// @param range_length The position of the first item to be reordered.
    /// @param range_start The position where the items should be inserted.
    /// @param insert_before The amount of items to be reordered.
    /// @return response object containing http status code and reply
    response update_playlist_items(const char* playlist_id, int size,  const char** uris = nullptr, int range_length = 1, int range_start = 0, int insert_before = 1 );
    /// @brief Add items to a playlist
    /// @param playlist_id Id of the playlist
    /// @param size Size of uris array
    /// @param uris Array of Spotify URIs of the items to add
    /// @param position The position to insert the items, a zero-based index
    /// @return response object containing http status code and reply
    response add_items_to_playlist(const char* playlist_id, int size, const char** uris, int position = 0);
    /// @brief Remove items from a playlist
    /// @param playlist_id Id of the playlist
    /// @param size Size of uris array
    /// @param uris Array of Spotify URIs of the items to remove
    /// @return response object containing http status code and reply
    response remove_playlist_items(const char* playlist_id, int size,  const char** uris);
    /// @brief Get a list of users playlists
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_current_users_playlists(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief get a users playlist
    /// @param user_id Id of the user
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_user_playlists(const char* user_id,int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Create a playlist
    /// @param user_id Id of the user
    /// @param name Name of the playlist
    /// @param is_public Set the playlist to public or not
    /// @param is_collaborative Set the playlist to collaborative or not
    /// @param description Description of the playlist
    /// @return response object containing http status code and reply
    response create_playlist(const char* user_id, const char* name, bool is_public, bool is_collaborative, const char* description);
    /// @brief Get a list of Spotify featured playlists
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param timestamp A timestamp in ISO 8601 format: yyyy-MM-ddTHH:mm:ss if ommited current utc time is used
    /// @param country An ISO 3166-1 alpha-2 country code, Provide this to ensure that the category exists for a particular country.
    /// @param locale The desired language, consisting of an ISO 639-1 language code and an ISO 3166-1 alpha-2 country code, joined by an underscore, if ommited the response defaults to American English
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_featured_playlists( int limit = 10, int offset  = 0, const char* timestamp  = nullptr, const char* country = nullptr, const char* locale = nullptr, JsonDocument filter = JsonDocument());
    /// @brief Get a list of Spotify playlists tagged with a particular category.
    /// @param category_id Category ID can be got from get_several_browse_categories
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param country The country: an ISO 3166-1 alpha-2 country code, Provide this to ensure that the category exists for a particular country.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_category_playlists(const char* category_id, int limit = 10, int offset = 0, const char* country = nullptr, JsonDocument filter = JsonDocument());
    /// @brief Get a cover image of a playlist
    /// @param playlist_id Id of the playlist
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_playlist_cover_image(const char* playlist_id, JsonDocument filter = JsonDocument());
    /// @brief Upload a custom cover image of a playlist
    /// @param playlist_id Id of the playlist
    /// @param data Image data
    /// @param market An ISO 3166-1 alpha-2 country code, Provide this parameter if you want to apply Track Relinking
    /// @return response object containing http status code and reply
    response add_custom_playlist_cover_image(const char* playlist_id, const char* data);
    #endif
  #ifndef DISABLE_SEARCH
    /// @brief Search for an item
    /// @param q Search query keywords and optional field filters and operators
    /// @param type_size Number of item types in type array
    /// @param type An array of item types to search across
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param market An ISO 3166-1 alpha-2 country code or the string from_token. Provide this parameter if you want the list of returned items to be relevant to a particular country.
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response search(const char* q,int type_size , const char** type , int limit = 10, int offset = 0, const char* market = nullptr, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_SHOWS
    /// @brief Get Spotify information for a single show
    /// @param show_id Spotify ID of the show
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_show(const char* show_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple shows
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_shows(int size, const char** show_ids, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information about a show's episodes
    /// @param show_id Spotify ID of the show
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_show_episodes(const char* show_id, int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Get users saved shows
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_users_saved_shows(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Save one or more shows to the current user's  music library
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @return response object containing http status code and reply
    response save_shows_for_current_user(int size,  const char** show_ids);
    /// @brief Remove one or more shows from the current user's  music library
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @return response object containing http status code and reply
    response remove_shows_for_current_user(int size,  const char** show_ids);
    /// @brief Check if one or more shows is already saved in the current Spotify user's  music library
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_users_saved_shows(int size,  const char** show_ids, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_TRACKS
    /// @brief Get Spotify information for a single track
    /// @param track_id Spotify ID of the track
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_track(const char* track_id, JsonDocument filter = JsonDocument());
    /// @brief Get Spotify information for multiple tracks
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_several_tracks(int size,  const char** track_ids, JsonDocument filter = JsonDocument());
    /// @brief Get users saved tracks
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_user_saved_tracks(int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Save one or more tracks to the current user's  music library
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response save_tracks_for_current_user(int size,  const char** track_ids);
    /// @brief Remove one or more tracks from the current user's  music library
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response remove_user_saved_tracks(int size,  const char** track_ids);
    /// @brief Check if one or more tracks is already saved in the current Spotify user's  music library
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_user_saved_tracks(int size,  const char** track_ids, JsonDocument filter = JsonDocument());
    /// @brief Get audio features for multiple tracks
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_tracks_audio_features(int size,  const char** track_ids, JsonDocument filter = JsonDocument());
    /// @brief Get audio features for a single track
    /// @param track_id Spotify ID of the track
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_track_audio_features(const char* track_id, JsonDocument filter = JsonDocument());
    /// @brief Get audio analysis for a single track
    /// @param track_id Spotify ID of the track
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_track_audio_analysis(const char* track_id, JsonDocument filter = JsonDocument());
    /// @brief Get a list of new album releases featured in Spotify
    /// @param recom Recommendation object containing atleast one seed
    /// @param limit The maximum number of items to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_recommendations(recommendations& recom, int limit = 10, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_USER    
    /// @brief Get detailed profile information about the current user (including the current user's username)
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_current_user_profile(JsonDocument filter = JsonDocument());
    /// @brief Get users top items
    /// @param type The type of item to get, Valid values are: artists or tracks
    /// @param time_range Over what time frame the affinities are computed, Valid values are: long_term, medium_term, short_term
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_user_top_items(const char* type, const char* time_range = "medium_term", int limit = 10, int offset = 0, JsonDocument filter = JsonDocument());
    /// @brief Get a users profile
    /// @param user_id Id of the user
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_user_profile(const char* user_id, JsonDocument filter = JsonDocument());
    /// @brief Follow a playlist
    /// @param playlist_id The Id of the playlist
    /// @param is_public If true the playlist will be included in user's public playlists, if false it will remain private.
    /// @return response object containing http status code and reply
    response follow_playlist(const char* playlist_id, bool is_public);
    /// @brief Unfollow a playlist
    /// @param playlist_id The Id of the playlist
    /// @return response object containing http status code and reply
    response unfollow_playlist(const char* playlist_id);
    /// @brief get users followed artists
    /// @param after The last artist ID retrieved from the previous request(Optional)
    /// @param type The ID type, currently only artist is supported
    /// @param limit The maximum number of items to return
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response get_followed_artists(const char* after = nullptr, const char* type = "artist", int limit = 10, JsonDocument filter = JsonDocument());
    /// @brief Follow artists or users
    /// @param type The ID type, artist or user 
    /// @param size Number of artist or user ids in artist_user_ids array
    /// @param artist_user_ids Array of Spotify IDs of the artists or users
    /// @return response object containing http status code and reply
    response follow_artists_or_users(const char* type, int size, const char** artist_user_ids);
    /// @brief Unfollow artists or users
    /// @param type The ID type, artist or user
    /// @param size Number of artist or user ids in artist_user_ids array
    /// @param artist_user_ids Array of Spotify IDs of the artists or users
    /// @return response object containing http status code and reply
    response unfollow_artists_or_users(const char* type, int size,  const char** artist_user_ids);
    /// @brief Check if users follow artists or users
    /// @param type The ID type, artist or user
    /// @param size Number of artist or user ids in artist_user_ids array
    /// @param artist_user_ids Array of Spotify IDs of the artists or users
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_if_user_follows_artists_or_users(const char* type, int size, const char** artist_user_ids, JsonDocument filter = JsonDocument());
    /// @brief Check if users follow a playlist
    /// @param playlist_id The ID of the playlist
    /// @param size Number of user ids in user_ids array
    /// @param user_ids Array of Spotify IDs of the users
    /// @param filter JsonDocument containing the fields to filter(Optional, returns all fields if not provided)
    /// @return response object containing http status code and reply
    response check_if_users_follow_playlist(const char* playlist_id, int size, const char** user_ids, JsonDocument filter = JsonDocument());
    #endif
  #ifndef DISABLE_SIMPLIFIED
    /// @brief Get Current track name
    /// @return Current track name as String
    String current_track_name();
    /// @brief Get Current track id
    /// @return Current track id as String
    String current_track_id();
    /// @brief Get Current device id
    /// @return Current device id as String
    String current_device_id();
    /// @brief Get Current artist names
    /// @return Current artist names as String
    String current_artist_names();
    /// @brief Get Current device id
    /// @param char array to store device id
    /// @return Current device id as pointer to char array
    char* current_device_id(char* device_id);
    /// @brief Get Current track id
    /// @param char array to store track id
    /// @return Current track id as pointer to char array
    char* current_track_id(char* track_id);
    /// @brief Get Current track name
    /// @param char array to store track name
    /// @return Current track name as pointer to char array
    char* current_track_name(char* track_name);
    /// @brief Get Current artist names
    /// @param char array to store artist names
    /// @return Current artist names as pointer to char array
    char* current_artist_names(char* artist_names);
    /// @brief Get if device is playing
    /// @return true if device is playing
    bool is_playing();
    /// @brief Get if it is possible to modify volume on current device
    /// @return true if it is possible to modify volume on current device
    bool volume_modifyable();
  /// @brief Get current album cover link
    /// @param int image array position
    /// @return Current album cover link as String
    String get_current_album_image_url(int image_int);
    #endif
    /// @brief Convert ID to URI
    /// @param id ID to convert
    /// @param type Type of ID
    /// @return URI as char
    char convert_id_to_uri(const char* id, const char* type);
    /// @brief Convert ID to URI
    /// @param id ID to convert
    /// @param type Type of ID
    /// @param uri char array to store URI
    /// @return URI as pointer to char array
    char* convert_id_to_uri(const char* id, const char* type, char* uri); 
    /// @brief Get the users Tokens
    /// @return a user_tokens object containig the id, secret and refresh_token
    user_tokens get_user_tokens();
    /// @brief Checks wheter an access token has already been received
    /// @return True if there is already an acess token
    bool has_access_token();
    /// @brief Gets an acess token
    /// @return True if the request was successfull
    bool get_access_token();
    /// @brief Get refresh token from auth code
    /// @param auth_code Auth code recieved by logging into Spotify
    /// @param redirect_uri URI passed as redirect uri on the auth code request
    /// @return True if successfull
    bool get_refresh_token(const char* auth_code, const char* redirect_uri);
    /// @brief Destroy Object and free used memory
    void end();
    

  private:
  #ifndef DISABLE_WEB_SERVER
    WebServer* _server;
    /// @brief Root login Page
    void server_on_root();
    /// @brief Response from login page
    void server_on_response();
    /// @brief Refresh token login Page
    void server_on_refresh();
    /// @brief Currying function for refresh login page
    friend std::function<void()> callback_fn_refresh(Spotify *spotify);
    /// @brief Currying function for root login page
    friend std::function<void()> callback_fn_root(Spotify *spotify);
    /// @brief Currying function for root login page response
    friend std::function<void()> callback_fn_response(Spotify *spotify);
    /// @brief Sets up server routes  
    void server_routes();
  #else
    void* _server;
  #endif
    /// @brief HTTPS client
    WiFiClientSecure _client;
    /// @brief Host for Tokens
    const char* _host = "api.spotify.com";
    /// @brief Base url for API requests
    const char* _base_url = "https://api.spotify.com/v1/";
    /// @brief Host for Tokens
    const char* _token_host = "accounts.spotify.com";
    /// @brief Maximum number of items in one request
    static const int _max_num_items = 20;
    /// @brief Maximum size of char array(35 been the size of a uri + comma + 150 as buffer for url etc.)
    static const int _max_char_size = 35*_max_num_items + 150;
    /// @brief Size of a uri
    static const int _size_of_uri = 45;
    /// @brief Size of an id
    static const int _size_of_id = 25;
    /// @brief True if credentials will be set during runtime
    bool _no_credentials = false;
    /// @brief Maximum number of items in one request
    int _max_num_retry = 3;
    /// @brief Users set redirect uri
    char _redirect_uri[100] = "";
    /// @brief Users refresh token
    char _refresh_token[300] = "";
    /// @brief user auth code
    char _auth_code[800] = "";
    /// @brief Users set client id
    char _client_id[100] = "";
    /// @brief Users set client secret
    char _client_secret[100] = "";
    /// @brief Current number of retries
    int _retry;
    /// @brief Timeout for HTTP etc. in Millisecods
    int _timeout = 1000;
    /// @brief Debug mode
    bool _debug_on;
    /// @brief port
    int _port;
    /// @brief Access token
    char  _access_token[400]; 
    /// @brief Get Access Token with refresh token
    /// @return true if token was successfully retrieved
    bool get_token();
    /// @brief Sends base headers for token request
    /// @param payload Payload to send
    /// @return true if request was successful
    bool token_base_req(String payload);
    /// @brief Checks if http code is valid
    /// @param code Http code to check
    /// @return True if code is valid
    bool valid_http_code(int code);
    /// @brief Process headers of response
    /// @return Http code
    header_resp process_headers();
    /// @brief Process response
    /// @param filter Filter to apply to response
    /// @return JsonDocument containing response
    JsonDocument process_response(header_resp header, JsonDocument filter = JsonDocument());
    /// @brief Initialize response object
    /// @param response_obj Response object to initialize
    void init_response(response* response_obj);
    /// @brief Make request to Spotify API
    /// @param rest_url URL to make request to
    /// @param type Type of request
    /// @param payload_size Size of payload
    /// @param payload Payload to send
    response RestApi(const char* rest_url, const char* type, int payload_size = 0, const char* payload = nullptr, JsonDocument filter = JsonDocument());
    /// @brief Make PUT request to Spotify API
    /// @param rest_url URL to make request to
    /// @param payload_size Size of payload
    /// @param payload Payload to send
    /// @return Response object containing http status code and reply
    response RestApiPut(const char* rest_url, int payload_size = 0, const char* payload = nullptr);
    /// @brief Make POST request to Spotify API
    /// @param rest_url URL to make request to
    /// @param payload_size Size of payload
    /// @param payload Payload to send
    /// @return Response object containing http status code and reply
    response RestApiPost(const char* rest_url, int payload_size = 0, const char* payload = nullptr);
    /// @brief Make DELETE request to Spotify API
    /// @param rest_url URL to make request to
    /// @param payload Payload to send
    /// @return Response object containing http status code and reply
    response RestApiDelete(const char* rest_url, const char* payload = nullptr);
    /// @brief Make GET request to Spotify API
    /// @param rest_url URL to make request to
    /// @return Response object containing http status code and reply
    response RestApiGet(const char* rest_url,  JsonDocument filter = JsonDocument());
    /// @brief Convert array of chars to one char array, seperated by comma
    /// @param size Size of array
    /// @param array Array to convert
    /// @param result Array to store result
    /// @return Pointer to result array
    char* array_to_char(int size,  const char** array, char* result);
    /// @brief Convert array of chars to one json array
    /// @param size Size of array
    /// @param array Array to convert
    /// @param data Array to store result
    /// @param data_size Size of data array
    /// @return Pointer to data array
    void array_to_json_array(int size, const char** array, char* data, int data_size = _max_char_size);
    
  #ifndef DISABLE_TRACKS
    /// @brief Check if recommendation value is valid
    /// @param param Float value to check
    /// @return Bool if value is valid
    bool is_valid_value(float param);
    /// @brief Check if recommendation value is valid
    /// @param param Int value to check
    /// @return Bool if value is valid
    bool is_valid_value(int param);
    /// @brief Populate recommendation char values
    /// @param map Map to populate
    /// @param recom recommendation object
    /// @return Void
    void populate_char_values(std::map<const char*, char*>& map, recommendations& recom);
    /// @brief Populate recommendation float values
    /// @param map Map to populate
    /// @param recom recommendation object
    /// @return Void
    void populate_float_values(std::map<const char*, float>& map, recommendations& recom);
  #endif
    /// @brief Extract endpoint from url with regex
    /// @param rest_url URL to extract endpoint from
    /// @return Endpoint
    String extract_endpoint(const char* rest_url);
    /// @brief Root CA for Spotify API
    const char* _spotify_root_ca PROGMEM= \
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
    const char* _credentials_input PROGMEM = R"=====(
    <HTML>
        <HEAD>
            <TITLE>Enter Credentials</TITLE>
        </HEAD>
        <BODY>
            <h1>Enter your Credentials</h1>
            <p>If you don't have a refresh token leave empty</p>
            <form action="/get">
                Client ID: <input type="text" name="id">
                <p></p>
                Client Secret: <input type="text" name="secret">
                <p></p>
                Refreshtoken: <input type="text" name="token">
                <input type="submit" value="Submit">
            </form>
        </BODY>
    </HTML>)=====";
    const char* _login_page PROGMEM = R"=====(
    <HTML>
      <HEAD>
        <TITLE>ESP Spotify Login</TITLE>
      </HEAD>
      <BODY>
        <CENTER>
          <H1>Spotify Login</H1>
          <a href="https://accounts.spotify.com/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=ugc-image-upload playlist-read-collaborative playlist-modify-private playlist-modify-public playlist-read-private user-read-playback-position user-read-recently-played user-top-read user-modify-playback-state user-read-currently-playing user-read-playback-state user-read-private user-read-email user-library-modify user-library-read user-follow-modify user-follow-read streaming app-remote-control">Log in to spotify</a>
        </CENTER>
      </BODY>
    </HTML>
    )=====";
};
#endif
