#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "SpotifyESP32.h"

/*const char* ssid = "Rebweg10D";
const char* password = "Bitte_eintreten";*/
const char* ssid = "WLAN-SVRZWX";
const char* password = "IG?gl57%";
char* client_id = "fc0799459cf04f1aab6bb3f861964e77";
char* client_secret = "9ca64f76f7b54ecdaf3ae78585ff8b0f";
char* redirect_uri = "http://localhost:8080";
char* refresh_token="AQAichrGMfjjDHYWIAENJYyWoi_KpzLZ93_HSS30J8zULeuHRPmF9-Wh3aSEd5ju8tloWGDZ9GJodjRH51PRUPvTqPg855b3fdmQ8JifmdNAEfSapfhkihTKKXveJxie00o";

using namespace Spotify_types;

Spotify sp(refresh_token, redirect_uri, client_id, client_secret, true);
String song_id = "2NTCi4wGypj56t843jb3Mt";
String track_id = "2NTCi4wGypj56t843jb3Mt";
char* track_ids[] = {"7ouMYWpwJ422jRcDASZB7P","4VqPOruhp5EdPBeR92t6lQ","2takcwOaAZWiXQijPHIx7B"};

String album_id = "7iLuHJkrb9KHPkMgddYigh";
char* album_ids[] = {"7iLuHJkrb9KHPkMgddYigh","4KAtLRVIfB0bKnRY01dveY","2SxoeF005n621Jca66RRdu", nullptr};

String artist_id = "7orlzf5LTqSnCzURkZFebN";
char* artist_ids[] = {"7orlzf5LTqSnCzURkZFebN","0lNJF6sbrXXPubqKkkyK23","3JsMj0DEzyWc0VDlHuy9Bx",nullptr};

String audiobook_id = "7iHfbu1YPACw6oZPAFJtqe";
char* audiobook_ids[] = {"7iHfbu1YPACw6oZPAFJtqe","1HGw3J3NxZO1TP1BTtVhpZ","7iHfbu1YPACw6oZPAFJtqe"};

String chapter_id = "0IsXVP0JmcB2adSE338GkK";
char* chapter_ids[] = {"0IsXVP0JmcB2adSE338GkK","3ZXb8FKZGU0EHALYX6uCzU","0D5wENdkdwbqlrHoaJ9g29"};

String country = "CH";
String locale  = "de_CH";
String category_id = "dinner";

String episode_id = "3UHFkXFDAHr7cBlRoUmdiY";
char* episode_ids[] = {"4H4ZXQ07SehfQDAImiHOXF","3UHFkXFDAHr7cBlRoUmdiY"};

String playlist_id = "5ZJAomiAtiow9nTloNl01E";
String fields = "items(track(name,href,album(name,href)))";


char* user_ids[] = {"adix3gjuq0g570rwufhfcw89o"};
String user_id = "adix3gjuq0g570rwufhfcw89o";
String show_id = "5CfCWKI5pZ28U0uOzXkDHe";
char* show_ids[] = {"5CfCWKI5pZ28U0uOzXkDHe","5as3aKmN2k11yfDDDSrvaZ"};

char* artist_user_ids[] = {"2CIMQHirSU0MQqyYHq0eOx","57dN52uHvrHOxijzpIgu3E","1vCWHaC5f2uS3yhpwWbIA6"};
void setup() {
  Serial.begin(115200);
  connectToWifi();
  //Create a list of track uris from track ids-
  char* track_uri[] = {strdup(sp.convert_id_to_uri(song_id, TYPE_TRACK).c_str())};
  Serial.println(track_uri[0]);
  Serial.println("update_playlist_items: ");
  print_response(sp.update_playlist_items(playlist_id, track_uri));

  test_playlist();
  test_shows();
  test_tracks();
  test_users();
}
void loop() {
  // put your main code here, to run repeatedly:

}

void connectToWifi(){
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}
//working
void test_player(){
  Serial.print("Currently Playing: ");
  print_response(sp.currently_playing());

  Serial.print("Play track: ");
  print_response(sp.play_uri(sp.convert_id_to_uri(song_id,TYPE_TRACK)));

  Serial.print("Pause: ");
  print_response(sp.pause_playback());

  Serial.print("Play: ");
  print_response(sp.start_playback());

  Serial.print("Skip: ");
  print_response(sp.skip());

  Serial.print("Previous: ");
  print_response(sp.previous());

  Serial.print("Available Devices: ");
  print_response(sp.available_devices());

  Serial.print("Playback state: ");
  print_response(sp.current_playback_state());

  Serial.print("Recently played: ");
  print_response(sp.recently_played_tracks(1));

  Serial.print("Seek to pos: ");
  print_response(sp.seek_to_position(500));

  Serial.print("Add to queue: ");
  print_response(sp.add_to_queue(sp.convert_id_to_uri(song_id,TYPE_TRACK)));
  

  Serial.print("Get Queue: ");
  print_response(sp.get_queue());

  Serial.print("Repeat: ");
  print_response(sp.repeat_mode(REPEAT_CONTEXT));

  Serial.print("Shuffle off: ");
  print_response(sp.shuffle(SHUFFLE_OFF));

  Serial.print("Transfer Playback: ");
  print_response(sp.transfer_playback(sp.current_device_id()));

  Serial.print("Set Volume: ");
  print_response(sp.set_volume(50));
}
//working
void test_albums(){
  Serial.print("Get Album: ");
  print_response(sp.get_album(album_id));

  Serial.print("Get Albums: ");
  print_response(sp.get_albums(album_ids));

  Serial.print("Get Album Tracks: ");
  print_response(sp.get_album_tracks(album_id, 1, 0));

  Serial.print("Get Saved Albums: ");
  print_response(sp.get_users_saved_albums(1, 0));

  Serial.print("Remove Saved Albums: ");
  print_response(sp.remove_users_saved_albums(album_ids));

  Serial.print("Save Albums: ");
  print_response(sp.save_albums_for_current_user(album_ids)); 
 
  Serial.print("Check if Album Saved: ");
  print_response(sp.check_useres_saved_albums(album_ids));

  Serial.print("Get Releases: ");
  print_response(sp.get_new_releases("CH", 1, 0));
}
//working
void test_artist(){
  Serial.println("Get Artist: ");
  print_response(sp.get_artist(artist_id));
  Serial.println("Get Artists: ");
  print_response(sp.get_several_artists(artist_ids));
  Serial.println("Get Artist Albums: ");
  print_response(sp.get_artist_albums(artist_id, GROUP_ALBUM +","+GROUP_SINGLE, 1, 0));
  Serial.println("Get Artist top Tracks: ");
  print_response(sp.get_artist_top_tracks(artist_id, country));
  Serial.println("Get Artists related artists: ");
  print_response(sp.get_artist_related_artist(artist_id));
}
//Working, only available in US, UK, New Zealand, Australia
void test_audiobooks(){
  //400
  Serial.println("Get Audibook: ");
  print_response(sp.get_audiobook(audiobook_id));
  Serial.println("Get Audibooks: ");
  print_response(sp.get_several_audiobooks(audiobook_ids));
  //500
  Serial.println("Get Audibook Chapters: ");
  print_response(sp.get_audiobook_chapters(audiobook_id, 1, 0));
  //400
  Serial.println("Get Users Audibooks: ");
  print_response(sp.get_users_audiobooks(0, 1));
  Serial.println("Save Audibook for user: ");
  print_response(sp.save_audiobooks_for_current_user(audiobook_ids));
  Serial.println("Check audiobooks: ");
  print_response(sp.check_users_saved_audiobooks(audiobook_ids));
  Serial.println("Remove Audiobooks: ");
  print_response(sp.remove_audiobooks_for_current_user(audiobook_ids));
}
//Working, only available in US, UK, New Zealand, Australia
void test_chapters(){
  //404
  Serial.print("get chapter: ");
  print_response(sp.get_chapter(chapter_id));
  Serial.print("get chapter: ");
  print_response(sp.get_several_chapters(chapter_ids));
}
//working
void test_categories(){
  Serial.println("get_several_browse_categories: ");
  print_response(sp.get_several_browse_categories(country, locale));

  Serial.println("get_single_browse_category: ");
  print_response(sp.get_single_browse_category(category_id, country, locale));
}
//working
void test_episodes(){
  Serial.println("get_episode: ");
  print_response(sp.get_episode(episode_id));

  Serial.println("get_several_episodes: ");
  print_response(sp.get_several_episodes(episode_ids));

  Serial.println("get_users_saved_episodes: ");
  print_response(sp.get_users_saved_episodes());

  Serial.println("save_episodes_for_current_user: ");
  print_response(sp.save_episodes_for_current_user(episode_ids));

  Serial.println("check_users_saved_episodes: ");
  print_response(sp.check_users_saved_episodes(episode_ids));

  Serial.println("remove_episodes_for_current_user: ");
  print_response(sp.remove_episodes_for_current_user(episode_ids));


}
//working
void test_genres(){
  Serial.println("get_available_genre_seeds: ");
  print_response(sp.get_available_genre_seeds());
}
//working
void test_markets(){
  Serial.println("get_available_markets: ");
  print_response(sp.get_available_markets());
}
//working
void test_playlist(){
  Serial.println("get_playlist: ");
  print_response(sp.get_playlist(playlist_id, fields));
  
  Serial.println("change_playlist_details: ");
  print_response(sp.change_playlist_details(playlist_id, "Hello", false, false, "WTF!!"));

  Serial.println("get_playlist_items: ");
  print_response(sp.get_playlist_items(playlist_id, fields));

  String track_uri_str = sp.convert_id_to_uri(song_id, TYPE_TRACK);
  char* track_uri[] = {strdup(track_uri_str.c_str())};
  Serial.println("update_playlist_items: ");
  print_response(sp.update_playlist_items(playlist_id, track_uri ));
  Serial.println("add_items_to_playlist: ");
  print_response(sp.add_items_to_playlist(playlist_id, track_uri, 1));
  Serial.println("remove_playlist_items: ");
  print_response(sp.remove_playlist_items(playlist_id, track_uri));

  Serial.println("get_current_users_playlists: ");
  print_response(sp.get_current_users_playlists());
  
  Serial.println("get_user_playlists: ");
  print_response(sp.get_user_playlists(user_id));

  Serial.println("create_playlist: ");
  print_response(sp.create_playlist(user_id, "Test", true, false, "no"));

  Serial.println("get_featured_playlists: ");
  print_response(sp.get_featured_playlists(country, locale, ""));

  Serial.println("get_category_playlists: ");
  print_response(sp.get_category_playlists("dinner", country));

  Serial.println("get_playlist_cover_image: ");
  print_response(sp.get_playlist_cover_image(playlist_id));

  Serial.println("add_custom_playlist_cover_image: ");
  print_response(sp.add_custom_playlist_cover_image(playlist_id, "/9j/2wCEABoZGSccJz4lJT5CLy8vQkc9Ozs9R0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0cBHCcnMyYzPSYmPUc9Mj1HR0dEREdHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR//dAAQAAf/uAA5BZG9iZQBkwAAAAAH/wAARCAABAAEDACIAAREBAhEB/8QASwABAQAAAAAAAAAAAAAAAAAAAAYBAQAAAAAAAAAAAAAAAAAAAAAQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwAAARECEQA/AJgAH//Z"));

}
//working
void test_search(){
  Serial.println("search: ");
  print_response(sp.search("hello+world", TYPE_TRACK));
}
//working
void test_shows(){
  Serial.println("get_show: ");
  print_response(sp.get_show(show_id));

  Serial.println("get_several_shows: ");
  print_response(sp.get_several_shows(show_ids));

  Serial.println("get_show_episodes: ");
  print_response(sp.get_show_episodes(show_id));

  Serial.println("get_users_saved_shows: ");
  print_response(sp.get_users_saved_shows());

  Serial.println("save_shows_for_current_user: ");
  print_response(sp.save_shows_for_current_user(show_ids));

  Serial.println("check_users_saved_shows: ");
  print_response(sp.check_users_saved_shows(show_ids));

  Serial.println("remove_shows_for_current_user: ");
  print_response(sp.remove_shows_for_current_user(show_ids));


}
//working
void test_tracks(){
  Serial.println("get_track: ");
  print_response(sp.get_track(track_id));

  Serial.println("get_several_tracks: ");
  print_response(sp.get_several_tracks(track_ids));

  Serial.println("get_user_saved_tracks: ");
  print_response(sp.get_user_saved_tracks());

  Serial.println("save_tracks_for_current_user: ");
  print_response(sp.save_tracks_for_current_user(track_ids));

  Serial.println("remove_user_saved_tracks: ");
  print_response(sp.remove_user_saved_tracks(track_ids));

  Serial.println("check_user_saved_tracks: ");
  print_response(sp.check_user_saved_tracks(track_ids));

  Serial.println("get_tracks_audio_features: ");
  print_response(sp.get_tracks_audio_features(track_ids));

  Serial.println("get_track_audio_features: ");
  print_response(sp.get_track_audio_features(track_id));

  Serial.println("get_track_audio_analysis: ");
  print_response(sp.get_track_audio_analysis(track_id));
}
//working
void test_users(){
  Serial.println("get_current_user_profile: ");
  print_response(sp.get_current_user_profile());

  Serial.println("get_user_top_items: ");
  print_response(sp.get_user_top_items(TOP_TYPE_ARTIST, TIME_RANGE_MEDIUM));

  Serial.println("get_user_profile: ");
  print_response(sp.get_user_profile(user_id));

  Serial.println("follow_playlist: ");
  print_response(sp.follow_playlist(playlist_id, true));

  Serial.println("unfollow_playlist: ");
  print_response(sp.unfollow_playlist(playlist_id));

  Serial.println("get_followed_artists: ");
  print_response(sp.get_followed_artists(""));

  Serial.println("follow_artists_or_users: ");
  print_response(sp.follow_artists_or_users(TYPE_ARTIST, artist_user_ids));

  Serial.println("unfollow_artists_or_users: ");
  print_response(sp.unfollow_artists_or_users(TYPE_ARTIST, artist_user_ids));

  Serial.println("check_if_user_follows_artists_or_users: ");
  print_response(sp.check_if_user_follows_artists_or_users(TYPE_ARTIST, artist_user_ids));
  
  Serial.println("check_if_users_follow_playlist: ");
  print_response(sp.check_if_users_follow_playlist(playlist_id, user_ids));

}

