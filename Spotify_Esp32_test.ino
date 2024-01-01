#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "SpotifyESP32.h"

/*const char* ssid = "Rebweg10D";
const char* password = "Bitte_eintreten";*/
const char ssid[] = "WLAN-SVRZWX";
const char password[] = "IG?gl57%";
const char* client_id = "fc0799459cf04f1aab6bb3f861964e77";
const char* client_secret = "9ca64f76f7b54ecdaf3ae78585ff8b0f";
const char* redirect_uri = "http://localhost:8080";
const char* refresh_token="AQAichrGMfjjDHYWIAENJYyWoi_KpzLZ93_HSS30J8zULeuHRPmF9-Wh3aSEd5ju8tloWGDZ9GJodjRH51PRUPvTqPg855b3fdmQ8JifmdNAEfSapfhkihTKKXveJxie00o";

using namespace Spotify_types;

Spotify sp(refresh_token, redirect_uri, client_id, client_secret, true);

char song_id[] = "2NTCi4wGypj56t843jb3Mt";
char* track_ids[] = {"7ouMYWpwJ422jRcDASZB7P","4VqPOruhp5EdPBeR92t6lQ","2takcwOaAZWiXQijPHIx7B"};

char album_id[] = "7iLuHJkrb9KHPkMgddYigh";
char* album_ids[] = {"7iLuHJkrb9KHPkMgddYigh","4KAtLRVIfB0bKnRY01dveY","2SxoeF005n621Jca66RRdu"};

char artist_id[] = "7orlzf5LTqSnCzURkZFebN";
char* artist_ids[] = {"7orlzf5LTqSnCzURkZFebN","0lNJF6sbrXXPubqKkkyK23","3JsMj0DEzyWc0VDlHuy9Bx"};

char audiobook_id[] = "7iHfbu1YPACw6oZPAFJtqe";
char* audiobook_ids[] = {"7iHfbu1YPACw6oZPAFJtqe","1HGw3J3NxZO1TP1BTtVhpZ","7iHfbu1YPACw6oZPAFJtqe"};

char chapter_id[] = "0IsXVP0JmcB2adSE338GkK";
char* chapter_ids[] = {"0IsXVP0JmcB2adSE338GkK","3ZXb8FKZGU0EHALYX6uCzU","0D5wENdkdwbqlrHoaJ9g29"};

char country[] = "CH";
char locale[]  = "de_CH";
char category_id[] = "dinner";

char episode_id[] = "3UHFkXFDAHr7cBlRoUmdiY";
char* episode_ids[] = {"4H4ZXQ07SehfQDAImiHOXF","3UHFkXFDAHr7cBlRoUmdiY"};

char playlist_id[] = "5ZJAomiAtiow9nTloNl01E";
char fields[] = "items(track(name,href,album(name,href)))";


char* user_ids[] = {"adix3gjuq0g570rwufhfcw89o"};
char user_id[] = "adix3gjuq0g570rwufhfcw89o";
char show_id[] = "5CfCWKI5pZ28U0uOzXkDHe";
char* show_ids[] = {"5CfCWKI5pZ28U0uOzXkDHe","5as3aKmN2k11yfDDDSrvaZ"};

char* artist_user_ids[] = {"2CIMQHirSU0MQqyYHq0eOx","57dN52uHvrHOxijzpIgu3E","1vCWHaC5f2uS3yhpwWbIA6"};
void setup() {
  Serial.begin(115200);
  connectToWifi();
  test_player();
  test_albums();
  test_artist();
  test_audiobooks();
  test_chapters();
  test_categories();
  test_episodes();
  test_genres();
  test_markets();
  test_playlist();
  test_search();
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

  char uri[SIZE_OF_URI];
  Serial.print("Play track: ");
  sp.convert_id_to_uri(song_id,TYPE_TRACK,uri);
  print_response(sp.start_resume_playback(uri, 0, 0));

  Serial.print("Pause: ");
  print_response(sp.pause_playback());

  Serial.print("Play: ");
  print_response(sp.start_resume_playback());

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
  print_response(sp.add_to_queue(uri));
  
  Serial.print("Get Queue: ");
  print_response(sp.get_queue());

  Serial.print("Repeat: ");
  print_response(sp.repeat_mode(REPEAT_CONTEXT));

  Serial.print("Shuffle off: ");
  print_response(sp.shuffle(SHUFFLE_OFF));

  char device_id[SIZE_OF_ID];
  Serial.print("Transfer Playback: ");
  print_response(sp.transfer_playback(sp.current_device_id(device_id)));

  Serial.print("Set Volume: ");
  print_response(sp.set_volume(50));
}
//working
void test_albums(){
  Serial.print("Get Album: ");
  print_response(sp.get_album(album_id));

  Serial.print("Get Albums: ");
  print_response(sp.get_albums(3,album_ids));

  Serial.print("Get Album Tracks: ");
  print_response(sp.get_album_tracks(album_id, 1, 0));

  Serial.print("Get Saved Albums: ");
  print_response(sp.get_users_saved_albums(1, 0));

  Serial.print("Remove Saved Albums: ");
  print_response(sp.remove_users_saved_albums(3,album_ids));

  Serial.print("Save Albums: ");
  print_response(sp.save_albums_for_current_user(3,album_ids)); 
 
  Serial.print("Check if Album Saved: ");
  print_response(sp.check_useres_saved_albums(3,album_ids));

  Serial.print("Get Releases: ");
  print_response(sp.get_new_releases("CH", 1, 0));
}
//working
void test_artist(){
  Serial.print("Get Artist: ");
  print_response(sp.get_artist(artist_id));
  Serial.print("Get Artists: ");
  print_response(sp.get_several_artists(3,artist_ids));
  Serial.print("Get Artist Albums: ");
  char* include_groups[] = {GROUP_ALBUM, GROUP_SINGLE};
  print_response(sp.get_artist_albums(artist_id, 2, include_groups, 1, 0));
  Serial.print("Get Artist top Tracks: ");
  print_response(sp.get_artist_top_tracks(artist_id, country));
  Serial.print("Get Artists related artists: ");
  print_response(sp.get_artist_related_artist(artist_id));
}
//Working, only available in US, UK, New Zealand, Australia
void test_audiobooks(){
  //400
  Serial.print("Get Audibook: ");
  print_response(sp.get_audiobook(audiobook_id));
  Serial.print("Get Audibooks: ");
  print_response(sp.get_several_audiobooks(3,audiobook_ids));
  //500
  Serial.print("Get Audibook Chapters: ");
  print_response(sp.get_audiobook_chapters(audiobook_id, 1, 0));
  //400
  Serial.print("Get Users Audibooks: ");
  print_response(sp.get_users_audiobooks(0, 1));
  Serial.print("Save Audibook for user: ");
  print_response(sp.save_audiobooks_for_current_user(3,audiobook_ids));
  Serial.print("Check audiobooks: ");
  print_response(sp.check_users_saved_audiobooks(3,audiobook_ids));
  Serial.print("Remove Audiobooks: ");
  print_response(sp.remove_audiobooks_for_current_user(3,audiobook_ids));
}
//Working, only available in US, UK, New Zealand, Australia
void test_chapters(){
  Serial.print("get chapter: ");
  print_response(sp.get_chapter(chapter_id));
  Serial.print("get chapter: ");
  print_response(sp.get_several_chapters(3,chapter_ids));
}
//working
void test_categories(){
  Serial.print("get_several_browse_categories: ");
  print_response(sp.get_several_browse_categories(country, locale));

  Serial.print("get_single_browse_category: ");
  print_response(sp.get_single_browse_category(category_id, country, locale));
}
//working
void test_episodes(){
  Serial.print("get_episode: ");
  print_response(sp.get_episode(episode_id));

  Serial.print("get_several_episodes: ");
  print_response(sp.get_several_episodes(2,episode_ids));

  Serial.print("get_users_saved_episodes: ");
  print_response(sp.get_users_saved_episodes());

  Serial.print("save_episodes_for_current_user: ");
  print_response(sp.save_episodes_for_current_user(2,episode_ids));

  Serial.print("check_users_saved_episodes: ");
  print_response(sp.check_users_saved_episodes(2,episode_ids));

  Serial.print("remove_episodes_for_current_user: ");
  print_response(sp.remove_episodes_for_current_user(2,episode_ids));


}
//working
void test_genres(){
  Serial.print("get_available_genre_seeds: ");
  print_response(sp.get_available_genre_seeds());
}
//working
void test_markets(){
  Serial.print("get_available_markets: ");
  print_response(sp.get_available_markets());
}
//working
void test_playlist(){
  Serial.print("get_playlist: ");
  print_response(sp.get_playlist(playlist_id, fields));
  
  Serial.print("change_playlist_details: ");
  print_response(sp.change_playlist_details(playlist_id, "Hello", false, false, "WTF!!"));

  Serial.print("get_playlist_items: ");
  print_response(sp.get_playlist_items(playlist_id, fields));

  char* track_uri[SIZE_OF_URI];
  sp.convert_id_to_uri(song_id,TYPE_TRACK,track_uri[0]);


  Serial.print("update_playlist_items: ");
  print_response(sp.update_playlist_items(playlist_id,1, track_uri, 1));

  Serial.print("add_items_to_playlist: ");
  print_response(sp.add_items_to_playlist(playlist_id,1, track_uri, 1));

  Serial.print("remove_playlist_items: ");
  print_response(sp.remove_playlist_items(playlist_id,1, track_uri));

  Serial.print("get_current_users_playlists: ");
  print_response(sp.get_current_users_playlists());
  
  Serial.print("get_user_playlists: ");
  print_response(sp.get_user_playlists(user_id));

  Serial.print("create_playlist: ");
  print_response(sp.create_playlist(user_id, "Test", true, false, "no"));
  
  Serial.print("get_featured_playlists: ");
  print_response(sp.get_featured_playlists(country, locale, ""));

  Serial.print("get_category_playlists: ");
  print_response(sp.get_category_playlists("dinner", country));

  Serial.print("get_playlist_cover_image: ");
  print_response(sp.get_playlist_cover_image(playlist_id));

  Serial.print("add_custom_playlist_cover_image: ");
  print_response(sp.add_custom_playlist_cover_image(playlist_id, "/9j/2wCEABoZGSccJz4lJT5CLy8vQkc9Ozs9R0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0cBHCcnMyYzPSYmPUc9Mj1HR0dEREdHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR//dAAQAAf/uAA5BZG9iZQBkwAAAAAH/wAARCAABAAEDACIAAREBAhEB/8QASwABAQAAAAAAAAAAAAAAAAAAAAYBAQAAAAAAAAAAAAAAAAAAAAAQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwAAARECEQA/AJgAH//Z"));

}
//working
void test_search(){
  Serial.print("search: ");
  print_response(sp.search("hello+world", TYPE_TRACK));
}
//working
void test_shows(){
  Serial.print("get_show: ");
  print_response(sp.get_show(show_id));

  Serial.print("get_several_shows: ");
  print_response(sp.get_several_shows(2,show_ids));

  Serial.print("get_show_episodes: ");
  print_response(sp.get_show_episodes(show_id));

  Serial.print("get_users_saved_shows: ");
  print_response(sp.get_users_saved_shows());

  Serial.print("save_shows_for_current_user: ");
  print_response(sp.save_shows_for_current_user(2,show_ids));

  Serial.print("check_users_saved_shows: ");
  print_response(sp.check_users_saved_shows(2,show_ids));

  Serial.print("remove_shows_for_current_user: ");
  print_response(sp.remove_shows_for_current_user(2,show_ids));


}
//working
void test_tracks(){
  Serial.print("get_track: ");
  print_response(sp.get_track(song_id));

  Serial.print("get_several_tracks: ");
  print_response(sp.get_several_tracks(3,track_ids));

  Serial.print("get_user_saved_tracks: ");
  print_response(sp.get_user_saved_tracks());

  Serial.print("save_tracks_for_current_user: ");
  print_response(sp.save_tracks_for_current_user(3,track_ids));

  Serial.print("remove_user_saved_tracks: ");
  print_response(sp.remove_user_saved_tracks(3,track_ids));

  Serial.print("check_user_saved_tracks: ");
  print_response(sp.check_user_saved_tracks(3,track_ids));

  Serial.print("get_tracks_audio_features: ");
  print_response(sp.get_tracks_audio_features(3,track_ids));

  Serial.print("get_track_audio_features: ");
  print_response(sp.get_track_audio_features(song_id));

  Serial.print("get_track_audio_analysis: ");
  print_response(sp.get_track_audio_analysis(song_id));
  char* track_seeds[] = {song_id};
  recommendations recom;
  recom.seed_tracks = track_seeds;
  recom.seed_tracks_size = 1;
  recom.max_energy = 0.5;
  Serial.print("get_recommendations: ");
  print_response(sp.get_recommendations(recom, 1));
}
//working
void test_users(){
  Serial.print("get_current_user_profile: ");
  print_response(sp.get_current_user_profile());

  Serial.print("get_user_top_items: ");
  print_response(sp.get_user_top_items(TOP_TYPE_ARTIST, TIME_RANGE_MEDIUM));

  Serial.print("get_user_profile: ");
  print_response(sp.get_user_profile(user_id));

  Serial.print("follow_playlist: ");
  print_response(sp.follow_playlist(playlist_id, true));

  Serial.print("unfollow_playlist: ");
  print_response(sp.unfollow_playlist(playlist_id));

  Serial.print("get_followed_artists: ");
  print_response(sp.get_followed_artists(""));

  Serial.print("follow_artists_or_users: ");
  print_response(sp.follow_artists_or_users(TYPE_ARTIST, 3,artist_user_ids));

  Serial.print("unfollow_artists_or_users: ");
  print_response(sp.unfollow_artists_or_users(TYPE_ARTIST, 3,artist_user_ids));

  Serial.print("check_if_user_follows_artists_or_users: ");
  print_response(sp.check_if_user_follows_artists_or_users(TYPE_ARTIST,3, artist_user_ids));
  
  Serial.print("check_if_users_follow_playlist: ");
  print_response(sp.check_if_users_follow_playlist(playlist_id, 3, user_ids));

}