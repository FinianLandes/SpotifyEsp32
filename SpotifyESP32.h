#ifndef SpotifyESP32
#define SpotifyESP32

#define ENABLE_PLAYER
#define ENABLE_ALBUM
#define ENABLE_ARTIST
#define ENABLE_AUDIOBOOKS
#define ENABLE_CATEGORIES
#define ENABLE_CHAPTERS
#define ENABLE_EPISODES
#define ENABLE_GENRES
#define ENABLE_MARKETS
#define ENABLE_PLAYLISTS
#define ENABLE_SEARCH
#define ENABLE_SHOWS
#define ENABLE_TRACKS
#define ENABLE_USER
#define ENABLE_SIMPIFIED

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



namespace Spotify_types {
  extern bool SHUFFLE_ON;
  extern bool SHUFFLE_OFF;
  extern char* REPEAT_OFF;
  extern char* REPEAT_TRACK;
  extern char* REPEAT_CONTEXT;
  extern char* TYPE_ALBUM;
  extern char* TYPE_ARTIST;
  extern char* TYPE_TRACK;
  extern char* TYPE_PLAYLIST;
  extern char* TOP_TYPE_ARTIST;
  extern char* TOP_TYPE_TRACKS;
  extern char* GROUP_ALBUM;
  extern char* GROUP_SINGLE;
  extern char* GROUP_APPEARS_ON;
  extern char* GROUP_COMPILATION;
  extern char* TIME_RANGE_SHORT;
  extern char* TIME_RANGE_MEDIUM;
  extern char* TIME_RANGE_LONG;
  extern int SIZE_OF_ID;
  extern int SIZE_OF_URI;

}

/// @brief Response object containing http status code and reply
typedef struct{
  int status_code;
  String reply;
} response;

/// @brief Recommendation object, used to create recommendations
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

/// @brief Print response object
/// @param response_obj Response object to print
void print_response(response response_obj);
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
  #ifdef ENABLE_PLAYER
    ///@brief Get information about the user's current playback state, including track, track progress, and active device.
    ///@return response object containing http status code and reply
    response currently_playing();
    ///@brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. 
    ///@param context_uri Spotify URI of the context to play (Required)
    ///@param offset Indicates from where in the context playback should start, Only works with albums or Playlists. NEEDS TO BE SET (0) IF ONLY URI IS PROVIDED
    ///@param position_ms Indicates from what position in the context playback should start in milliseconds(Optional)
    ///@param device_id Id of the device this command is targeting (Optional)
    ///@return response object containing http status code and reply
    response start_resume_playback(char* context_uri, int offset, int position_ms = 0, char* device_id = nullptr);
   ///@brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. 
    ///@param size Number of uris in uris array
    ///@param uris Array of Spotify URIs of the tracks to play
    ///@param device_id Id of the device this command is targeting (Optional)
    ///@return response object containing http status code and reply
    response start_resume_playback(int size, char ** uris ,char* device_id = nullptr);
    ///@brief Start or resume playback. If no device_id is provided, the user's currently active device is the target. Targets the currently playing context.
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
    response get_new_releases(int limit = 10, int offset = 0, char* country = nullptr);
    #endif
  #ifdef ENABLE_ARTIST
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
    ///@brief Get Spotify information for a single audiobook(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    ///@param audiobook_id Spotify ID of the audiobook
    ///@return response object containing http status code and reply
    response get_audiobook(char* audiobook_id);
    ///@brief Get Spotify information for multiple audiobooks(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    ///@param size Number of audiobook ids in audiobook_ids array
    ///@param audiobook_ids Array of Spotify IDs of the audiobooks
    ///@return response object containing http status code and reply
    response get_several_audiobooks(int size,  char** audiobook_ids);
    ///@brief Get Spotify information about an audiobook's chapters(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    ///@param audiobook_id Spotify ID of the audiobook
    ///@param limit The maximum number of items to return. Default: 10. Minimum: 1. Maximum: 50
    ///@param offset The index of the first chapter to return. Default: 0 (the first object). Use with limit to get the next set of chapters.
    ///@return response object containing http status code and reply
    response get_audiobook_chapters(char* audiobook_id, int limit = 10, int offset = 0);
    /// @brief Get users saved audiobooks(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_users_audiobooks(int limit = 10, int offset = 0);
    /// @brief Save one or more audiobooks to the current user's  music library(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response save_audiobooks_for_current_user(int size,  char** audiobook_ids);
    /// @brief Remove one or more audiobooks from the current user's  music library(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response remove_audiobooks_for_current_user(int size,  char** audiobook_ids);
    /// @brief Check if one or more audiobooks is already saved in the current Spotify user's  music library(Only Available in US, UK, Canada, Ireland, New Zealand and Australia)
    /// @param size Number of audiobook ids in audiobook_ids array
    /// @param audiobook_ids Array of Spotify IDs of the audiobooks
    /// @return response object containing http status code and reply
    response check_users_saved_audiobooks(int size,  char** audiobook_ids);
    #endif
  #ifdef ENABLE_CATEGORIES
    ///@brief Get a list of categories used to tag items in Spotify
    ///@param limit The maximum number of items to return
    ///@param offset The index of the first item to return
    ///@param country An ISO 3166-1 alpha-2 country code, if ommited the returned items will not be country-specific
    ///@param locale The desired language, consisting of an ISO 639-1 language code and an ISO 3166-1 alpha-2 country code, joined by an underscore, if ommited the response defaults to American English
    ///@return response object containing http status code and reply
    response get_several_browse_categories(int limit = 10, int offset = 0, char* country = nullptr, char* locale = nullptr);
    ///@brief Get a single category used to tag items in Spotify
    ///@param category_id Spotify category ID of the category
    ///@param country An ISO 3166-1 alpha-2 country code, if ommited the returned items will not be country-specific
    ///@param locale The desired language, consisting of an ISO 639-1 language code and an ISO 3166-1 alpha-2 country code, joined by an underscore, if ommited the response defaults to American English
    ///@return response object containing http status code and reply
    response get_single_browse_category(char* category_id, char* country = nullptr, char* locale = nullptr);
    #endif
  #ifdef ENABLE_CHAPTERS
    ///@brief Get Spotify information for a single chapter, Only Available in US, UK, Canada, Ireland, New Zealand and Australia
    ///@param chapter_id Spotify ID of the chapter
    ///@return response object containing http status code and reply
    response get_chapter(char* chapter_id);
    ///@brief Get Spotify information for multiple chapters, Only Available in US, UK, Canada, Ireland, New Zealand and Australia
    ///@param size Number of chapter ids in chapter_ids array
    ///@param chapter_ids Array of Spotify IDs of the chapters
    ///@return response object containing http status code and reply
    response get_several_chapters(int size,  char** chapter_ids);
    #endif
  #ifdef ENABLE_EPISODES
    ///@brief Get Spotify information for a single episode
    ///@param episode_id Spotify ID of the episode
    ///@return response object containing http status code and reply
    response get_episode(char* episode_id);
    ///@brief Get Spotify information for multiple episodes
    ///@param size Number of episode ids in episode_ids array
    ///@param episode_ids Array of Spotify IDs of the episodes
    ///@return response object containing http status code and reply
    response get_several_episodes(int size,  char** episode_ids);
    ///@brief Get users saved episodes
    ///@param limit The maximum number of items to return,
    ///@param offset The index of the first item to return
    ///@return response object containing http status code and reply
    response get_users_saved_episodes(int limit = 10, int offset = 0);
    ///@brief Save one or more episodes to the current user's  music library
    ///@param size Number of episode ids in episode_ids array
    ///@param episode_ids Array of Spotify IDs of the episodes
    ///@return response object containing http status code and reply
    response save_episodes_for_current_user(int size,  char** episode_ids);
    ///@brief Remove one or more episodes from the current user's  music library
    ///@param size Number of episode ids in episode_ids array
    ///@param episode_ids Array of Spotify IDs of the episodes
    ///@return response object containing http status code and reply
    response remove_episodes_for_current_user(int size,  char** episode_ids);
    ///@brief Check if one or more episodes is already saved in the current Spotify user's  music library
    ///@param size Number of episode ids in episode_ids array
    ///@param episode_ids Array of Spotify IDs of the episodes
    ///@return response object containing http status code and reply
    response check_users_saved_episodes(int size,  char** episode_ids);
    #endif
  #ifdef ENABLE_GENRES
    ///@brief Get a list of available genre seeds for recommendations
    ///@return response object containing http status code and reply
    response get_available_genre_seeds();
    #endif
  #ifdef ENABLE_MARKETS
    ///@brief Get a list of available markets for recommendations
    ///@return response object containing http status code and reply
    response get_available_markets();
    #endif
  #ifdef ENABLE_PLAYLISTS
    ///@brief Get Spotify information for a single playlist
    ///@param playlist_id Spotify ID of the playlist
    ///@param size Number of fields in fields array
    ///@param fields Array of fields to return, leave empty to return all fields
    ///@param additional_types A comma-separated list of item types that your client supports besides the default track type. Valid types are: track and episode.
    ///@return response object containing http status code and reply
    response get_playlist(char* playlist_id,int size = 0, char** fields = nullptr,int size_of_additional_types = 0, char ** additional_types = nullptr);
    /// @brief Change details of a playlist
    /// @param playlist_id ID of the playlist
    /// @param name Set the name of the playlist
    /// @param is_public Set the playlist to public or not
    /// @param is_collaborative Set the playlist to collaborative or not
    /// @param description Set the description of the playlist
    /// @return response object containing http status code and reply
    response change_playlist_details(char* playlist_id, char* name, bool is_public, bool is_collaborative, char* description);
    /// @brief Get Items of a playlist
    /// @param playlist_id Id of the playlist
    /// @param fields Filters for the query: a comma-separated list of the fields to return. If omitted, all fields are returned. For example, to get just the total number of items and the request limit: 
    /// @param limit Number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_playlist_items(char* playlist_id, char* fields, int limit = 10, int offset = 0);
    /// @brief Either reorder or replace items in a playlist depending on the request's parameters
    /// @param playlist_id Id of the playlist
    /// @param size Size of uris array
    /// @param uris Array of Spotify URIs this will overwrite all existing items in the playlist, If items should only be reordered pass nullptr
    /// @param range_length The position of the first item to be reordered.
    /// @param range_start The position where the items should be inserted.
    /// @param insert_before The amount of items to be reordered.
    /// @return response object containing http status code and reply
    response update_playlist_items(char* playlist_id, int size,  char** uris = nullptr, int range_length = 1, int range_start = 0, int insert_before = 1 );
    /// @brief Add items to a playlist
    /// @param playlist_id Id of the playlist
    /// @param size Size of uris array
    /// @param uris Array of Spotify URIs of the items to add
    /// @param position The position to insert the items, a zero-based index
    /// @return response object containing http status code and reply
    response add_items_to_playlist(char* playlist_id, int size,  char** uris, int position = 0);
    /// @brief Remove items from a playlist
    /// @param playlist_id Id of the playlist
    /// @param size Size of uris array
    /// @param uris Array of Spotify URIs of the items to remove
    /// @return response object containing http status code and reply
    response remove_playlist_items(char* playlist_id, int size,  char** uris);
    /// @brief Get a list of users playlists
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_current_users_playlists(int limit = 10, int offset = 0);
    /// @brief get a users playlist
    /// @param user_id Id of the user
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_user_playlists(char* user_id,int limit = 10, int offset = 0);
    /// @brief Create a playlist
    /// @param user_id Id of the user
    /// @param name Name of the playlist
    /// @param is_public Set the playlist to public or not
    /// @param is_collaborative Set the playlist to collaborative or not
    /// @param description Description of the playlist
    /// @return response object containing http status code and reply
    response create_playlist(char* user_id, char* name, bool is_public, bool is_collaborative, char* description);
    /// @brief Get a list of Spotify featured playlists
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param timestamp A timestamp in ISO 8601 format: yyyy-MM-ddTHH:mm:ss if ommited current utc time is used
    /// @param country An ISO 3166-1 alpha-2 country code, Provide this to ensure that the category exists for a particular country.
    /// @param locale The desired language, consisting of an ISO 639-1 language code and an ISO 3166-1 alpha-2 country code, joined by an underscore, if ommited the response defaults to American English
    /// @return response object containing http status code and reply
    response get_featured_playlists( int limit = 10, int offset  = 0, char* timestamp  = nullptr, char* country = nullptr, char* locale = nullptr);
    /// @brief Get a list of Spotify playlists tagged with a particular category.
    /// @param category_id Category ID can be got from get_several_browse_categories
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param country The country: an ISO 3166-1 alpha-2 country code, Provide this to ensure that the category exists for a particular country.
    /// @return response object containing http status code and reply
    response get_category_playlists(char* category_id, int limit = 10, int offset = 0, char* country = nullptr);
    /// @brief Get a cover image of a playlist
    /// @param playlist_id Id of the playlist
    /// @return response object containing http status code and reply
    response get_playlist_cover_image(char* playlist_id);
    /// @brief Upload a custom cover image of a playlist
    /// @param playlist_id Id of the playlist
    /// @param data Image data
    /// @param market An ISO 3166-1 alpha-2 country code, Provide this parameter if you want to apply Track Relinking
    /// @return response object containing http status code and reply
    response add_custom_playlist_cover_image(char* playlist_id, char* data);
    #endif
  #ifdef ENABLE_SEARCH
    /// @brief Search for an item
    /// @param q Search query keywords and optional field filters and operators
    /// @param type_size Number of item types in type array,needs to e set to 0 if limit, offset or market is used and type is not used
    /// @param type A comma-separated list of item types to search across, needs to be set to nullptr if limit, offset or market is used and type is not used
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @param market An ISO 3166-1 alpha-2 country code or the string from_token, Provide this parameter if you want to apply Track Relinking
    /// @return response object containing http status code and reply
    response search(char* q,int type_size = 0, char** type = nullptr, int limit = 10, int offset = 0, char* market = nullptr);
    #endif
  #ifdef ENABLE_SHOWS
    /// @brief Get Spotify information for a single show
    /// @param show_id Spotify ID of the show
    /// @return response object containing http status code and reply
    response get_show(char* show_id);
    /// @brief Get Spotify information for multiple shows
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @return response object containing http status code and reply
    response get_several_shows(int size,  char** show_ids);
    /// @brief Get Spotify information about a show's episodes
    /// @param show_id Spotify ID of the show
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_show_episodes(char* show_id, int limit = 10, int offset = 0);
    /// @brief Get users saved shows
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_users_saved_shows(int limit = 10, int offset = 0);
    /// @brief Save one or more shows to the current user's  music library
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @return response object containing http status code and reply
    response save_shows_for_current_user(int size,  char** show_ids);
    /// @brief Remove one or more shows from the current user's  music library
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @return response object containing http status code and reply
    response remove_shows_for_current_user(int size,  char** show_ids);
    /// @brief Check if one or more shows is already saved in the current Spotify user's  music library
    /// @param size Number of show ids in show_ids array
    /// @param show_ids Array of Spotify IDs of the shows
    /// @return response object containing http status code and reply
    response check_users_saved_shows(int size,  char** show_ids);
    #endif
  #ifdef ENABLE_TRACKS
    /// @brief Get Spotify information for a single track
    /// @param track_id Spotify ID of the track
    /// @return response object containing http status code and reply
    response get_track(char* track_id);
    /// @brief Get Spotify information for multiple tracks
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response get_several_tracks(int size,  char** track_ids);
    /// @brief Get users saved tracks
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_user_saved_tracks(int limit = 10, int offset = 0);
    /// @brief Save one or more tracks to the current user's  music library
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response save_tracks_for_current_user(int size,  char** track_ids);
    /// @brief Remove one or more tracks from the current user's  music library
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response remove_user_saved_tracks(int size,  char** track_ids);
    /// @brief Check if one or more tracks is already saved in the current Spotify user's  music library
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response check_user_saved_tracks(int size,  char** track_ids);
    /// @brief Get audio features for multiple tracks
    /// @param size Number of track ids in track_ids array
    /// @param track_ids Array of Spotify IDs of the tracks
    /// @return response object containing http status code and reply
    response get_tracks_audio_features(int size,  char** track_ids);
    /// @brief Get audio features for a single track
    /// @param track_id Spotify ID of the track
    /// @return response object containing http status code and reply
    response get_track_audio_features(char* track_id);
    /// @brief Get audio analysis for a single track
    /// @param track_id Spotify ID of the track
    /// @return response object containing http status code and reply
    response get_track_audio_analysis(char* track_id);
    /// @brief Get a list of new album releases featured in Spotify
    /// @param recom Recommendation object containing atleast one seed
    /// @param limit The maximum number of items to return
    /// @return response object containing http status code and reply
    response get_recommendations(recommendations& recom, int limit = 10);
    #endif
  #ifdef ENABLE_USER    
    /// @brief Get detailed profile information about the current user (including the current user's username)
    /// @return response object containing http status code and reply
    response get_current_user_profile();
    /// @brief Get users top items
    /// @param type The type of item to get, Valid values are: artists or tracks
    /// @param time_range Over what time frame the affinities are computed, Valid values are: long_term, medium_term, short_term
    /// @param limit The maximum number of items to return
    /// @param offset The index of the first item to return
    /// @return response object containing http status code and reply
    response get_user_top_items(char* type, char* time_range = "medium_term", int limit = 10, int offset = 0);
    /// @brief Get a users profile
    /// @param user_id Id of the user
    /// @return response object containing http status code and reply
    response get_user_profile(char* user_id);
    /// @brief Follow a playlist
    /// @param playlist_id The Id of the playlist
    /// @param is_public If true the playlist will be included in user's public playlists, if false it will remain private.
    /// @return response object containing http status code and reply
    response follow_playlist(char* playlist_id, bool is_public);
    /// @brief Unfollow a playlist
    /// @param playlist_id The Id of the playlist
    /// @return response object containing http status code and reply
    response unfollow_playlist(char* playlist_id);
    /// @brief get users followed artists
    /// @param after The last artist ID retrieved from the previous request
    /// @param type The ID type, currently only artist is supported
    /// @param limit The maximum number of items to return
    /// @return response object containing http status code and reply
    response get_followed_artists(char* after, char* type = "artist", int limit = 10);
    /// @brief Follow artists or users
    /// @param type The ID type, artist or user
    /// @param size Number of artist or user ids in artist_user_ids array
    /// @param artist_user_ids Array of Spotify IDs of the artists or users
    /// @return response object containing http status code and reply
    response follow_artists_or_users(char* type, int size,  char** artist_user_ids);
    /// @brief Unfollow artists or users
    /// @param type The ID type, artist or user
    /// @param size Number of artist or user ids in artist_user_ids array
    /// @param artist_user_ids Array of Spotify IDs of the artists or users
    /// @return response object containing http status code and reply
    response unfollow_artists_or_users(char* type, int size,  char** artist_user_ids);
    /// @brief Check if users follow artists or users
    /// @param type The ID type, artist or user
    /// @param size Number of artist or user ids in artist_user_ids array
    /// @param artist_user_ids Array of Spotify IDs of the artists or users
    /// @return response object containing http status code and reply
    response check_if_user_follows_artists_or_users(char* type, int size,  char** artist_user_ids);
    /// @brief Check if users follow a playlist
    /// @param playlist_id The ID of the playlist
    /// @param size Number of user ids in user_ids array
    /// @param user_ids Array of Spotify IDs of the users
    /// @return response object containing http status code and reply
    response check_if_users_follow_playlist(char* playlist_id, int size,  char** user_ids);
    #endif
  #ifdef ENABLE_SIMPIFIED
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
    #endif
    /// @brief Convert ID to URI
    /// @param id ID to convert
    /// @param type Type of ID
    /// @return URI as char
    char convert_id_to_uri(char* id, char* type);
    /// @brief Convert ID to URI
    /// @param id ID to convert
    /// @param type Type of ID
    /// @param uri char array to store URI
    /// @return URI as pointer to char array
    char* convert_id_to_uri(char* id, char* type, char* uri); 
    

  private:
    /// @brief Maximum number of items in one request
    static const int _max_num_items = 20;
    /// @brief Maximum size of char array(35 been the size of a uri + comma + 150 as buffer for url etc.)
    static const int _max_char_size = 35*_max_num_items + 150;
    /// @brief Size of a uri
    static const int _size_of_uri = 45;
    /// @brief Size of an id
    static const int _size_of_id = 25;
    /// @brief Maximum number of items in one request
    int _max_num_retry = 3;
    /// @brief Users refresh token
    const char* _refresh_token;
    /// @brief Users set redirect uri
    const char* _redirect_uri;
    /// @brief Users set client id
    const char* _client_id;
    /// @brief Users set client secret
    const char* _client_secret;
    /// @brief Current number of retries
    int _retry;
    /// @brief Debug mode
    bool _debug_on;
    /// @brief Access token
    String  _access_token; 
    /// @brief Get Access Token with refresh token
    /// @return Bool if token was successfully retrieved
    bool get_token();
    /// @brief Initialize response object
    /// @param response_obj Response object to initialize
    void init_response(response* response_obj);
    /// @brief Make PUT request to Spotify API
    /// @param rest_url URL to make request to
    /// @param payload_size Size of payload
    /// @param payload Payload to send
    /// @return Response object containing http status code and reply
    response RestApiPut(char* rest_url, int payload_size = 0, char* payload = nullptr);
    /// @brief Make POST request to Spotify API
    /// @param rest_url URL to make request to
    /// @param payload_size Size of payload
    /// @param payload Payload to send
    /// @return Response object containing http status code and reply
    response RestApiPost(char* rest_url, int payload_size = 0, char* payload = nullptr);
    /// @brief Make DELETE request to Spotify API
    /// @param rest_url URL to make request to
    /// @param payload Payload to send
    /// @return Response object containing http status code and reply
    response RestApiDelete(char* rest_url, char* payload = nullptr);
    /// @brief Make GET request to Spotify API
    /// @param rest_url URL to make request to
    /// @return Response object containing http status code and reply
    response RestApiGet(char* rest_url); 
    /// @brief Convert array of chars to one char array, seperated by comma
    /// @param size Size of array
    /// @param array Array to convert
    /// @param result Array to store result
    /// @return Pointer to result array
    char* array_to_char(int size,  char** array, char* result);
    /// @brief Convert array of chars to one json array
    /// @param size Size of array
    /// @param array Array to convert
    /// @param data Array to store result
    /// @param data_size Size of data array
    /// @return Pointer to data array
    void array_to_json_array(int size,  char** array, char* data, int data_size = _max_char_size);//Convert array of chars to one json array
  #ifdef ENABLE_TRACKS
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
    void populate_char_values(std::map<char*, char*>& map, recommendations& recom);
    /// @brief Populate recommendation float values
    /// @param map Map to populate
    /// @param recom recommendation object
    /// @return Void
    void populate_float_values(std::map<char*, float>& map, recommendations& recom);
  #endif
    /// @brief Extract endpoint from url with regex
    const char * extract_endpoint(const char* rest_url);
    /// @brief Root CA for Spotify API
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
