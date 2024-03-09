#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyESP32.h"
#include "config.h"
using namespace Spotify_types;

void connect_to_wifi();
void check_all();
void check_one(response r);
Spotify sp(CLIENT_ID, CLIENT_SECRET,REFRESH_TOKEN, false);

void setup() {
    Serial.begin(115200);
    connect_to_wifi();
    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.println("Authenticated");
    check_all();
    Serial.println("Done");
}

void loop() {
    // put your main code here, to run repeatedly:
}
void connect_to_wifi(){
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected to WiFi\n");
}
void check_all(){
    /*Serial.println("Player");
    check_one(sp.currently_playing());
    check_one(sp.start_resume_playback("spotify:track:6xaMXvpkmVow1RAinlthUT"));
    check_one(sp.pause_playback());
    check_one(sp.start_resume_playback());
    check_one(sp.skip());
    check_one(sp.previous());
    check_one(sp.available_devices());
    check_one(sp.current_playback_state());
    check_one(sp.recently_played_tracks());
    check_one(sp.seek_to_position(1000));
    check_one(sp.add_to_queue("spotify:track:6xaMXvpkmVow1RAinlthUT"));
    check_one(sp.get_queue());
    check_one(sp.repeat_mode(REPEAT_OFF));  
    check_one(sp.shuffle(SHUFFLE_ON));
    check_one(sp.transfer_playback("956f01ff586ece9abaa152dd26bc5cc565be5a22")); 
    check_one(sp.set_volume(100));
    Serial.println("Album");
    check_one(sp.get_album("22pMyI5Ra0xRPDpf21ZWNb"));
    char* albums[] = {"22pMyI5Ra0xRPDpf21ZWNb", "5AXd8wp7NNydXNzFAxlXmf"};
    check_one(sp.get_albums(2, albums));
    check_one(sp.get_album_tracks("22pMyI5Ra0xRPDpf21ZWNb"));
    check_one(sp.get_users_saved_albums());
    check_one(sp.remove_users_saved_albums(2, albums));
    check_one(sp.save_albums_for_current_user(2, albums));
    check_one(sp.check_useres_saved_albums(2, albums));
    check_one(sp.get_new_releases());
    Serial.println("Artist");
    check_one(sp.get_artist("7keGfmQR4X5w0two1xKZ7d"));
    char* artists[] = {"7keGfmQR4X5w0two1xKZ7d", "6LtXxYMIiKSy2EGHnz1f5j"};
    check_one(sp.get_several_artists(2, artists));
    char * group[] = {GROUP_ALBUM, GROUP_SINGLE, GROUP_APPEARS_ON, GROUP_COMPILATION};
    check_one(sp.get_artist_albums("7keGfmQR4X5w0two1xKZ7d", 4, group));
    check_one(sp.get_artist_top_tracks("7keGfmQR4X5w0two1xKZ7d", "DE"));
    check_one(sp.get_artist_related_artist("7keGfmQR4X5w0two1xKZ7d"));
    Serial.println("Browse");
    check_one(sp.get_several_browse_categories());
    check_one(sp.get_single_browse_category("dinner"));
    Serial.println("Episode");
    check_one(sp.get_episode("3mbWNOW6PwYGrXANw2JJDU"));
    char* episodes[] = {"3mbWNOW6PwYGrXANw2JJDU", "4hwEvITXPZvcvL8UCVEp6Q"};
    check_one(sp.get_several_episodes(2, episodes));
    check_one(sp.get_users_saved_episodes());
    check_one(sp.save_episodes_for_current_user(2, episodes));
    check_one(sp.remove_episodes_for_current_user(2, episodes));
    check_one(sp.check_users_saved_episodes(2, episodes));
    Serial.println("Markets and Genres");
    check_one(sp.get_available_genre_seeds());
    check_one(sp.get_available_markets());
    Serial.println("Playlists");
    check_one(sp.get_playlist("37i9dQZF1EQn4jwNIohw50"));
    check_one(sp.change_playlist_details("39Swnx8UEHw4FK9FlQHCov", "New Name", false, false, "New Description"));
    check_one(sp.get_playlist_items("37i9dQZF1EQn4jwNIohw50","items(added_by.id,track(name,href,album(name,href)))"));
    char* tracks[] = {"spotify:track:6xaMXvpkmVow1RAinlthUT", "spotify:track:6xaMXvpkmVow1RAinlthUT"};
    check_one(sp.update_playlist_items("39Swnx8UEHw4FK9FlQHCov", 2, tracks));
    check_one(sp.remove_playlist_items("39Swnx8UEHw4FK9FlQHCov", 2, tracks));
    check_one(sp.add_items_to_playlist("39Swnx8UEHw4FK9FlQHCov", 2, tracks));
    check_one(sp.get_current_users_playlists());
    check_one(sp.get_user_playlists("spotify"));
    check_one(sp.create_playlist("adix3gjuq0g570rwufhfcw89o","New Playlist", false,true, "New Description"));
    check_one(sp.get_featured_playlists());
    check_one(sp.get_category_playlists("dinner"));
    check_one(sp.get_playlist_cover_image("37i9dQZF1EQn4jwNIohw50"));
    check_one(sp.add_custom_playlist_cover_image("39Swnx8UEHw4FK9FlQHCov", "/9j/2wCEABoZGSccJz4lJT5CLy8vQkc9Ozs9R0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0cBHCcnMyYzPSYmPUc9Mj1HR0dEREdHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR//dAAQAAf/uAA5BZG9iZQBkwAAAAAH/wAARCAABAAEDACIAAREBAhEB/8QASwABAQAAAAAAAAAAAAAAAAAAAAYBAQAAAAAAAAAAAAAAAAAAAAAQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwAAARECEQA/AJgAH//Z"));
    Serial.println("Search");
    char* types[] = {TYPE_ALBUM, TYPE_ARTIST, TYPE_PLAYLIST, TYPE_TRACK};
    check_one(sp.search("remaster%2520track%3ADoxy%2520artist%3AMiles%2520Davis", 4, types));
    Serial.println("Shows");
    check_one(sp.get_show("5icRmO26ZsDWsNjUy5rssH"));
    char* shows[] = {"5icRmO26ZsDWsNjUy5rssH", "3wBfqov60qDZbEVjPHo0a8"};
    check_one(sp.get_several_shows(2, shows));
    check_one(sp.get_show_episodes("5icRmO26ZsDWsNjUy5rssH"));
    check_one(sp.get_users_saved_shows());
    check_one(sp.save_shows_for_current_user(2, shows));
    check_one(sp.remove_shows_for_current_user(2, shows));
    check_one(sp.check_users_saved_shows(2, shows));
    Serial.println("Tracks");
    check_one(sp.get_track("49A10weBfgwg4TUM3My7iv"));
    char* tracks[] = {"49A10weBfgwg4TUM3My7iv", "3qhlB30KknSejmIvZZLjOD"};
    check_one(sp.get_several_tracks(2, tracks));
    check_one(sp.get_user_saved_tracks());
    check_one(sp.save_tracks_for_current_user(2, tracks));
    check_one(sp.remove_user_saved_tracks(2, tracks));
    check_one(sp.check_user_saved_tracks(2, tracks));
    check_one(sp.get_tracks_audio_features(2, tracks));
    check_one(sp.get_track_audio_analysis("49A10weBfgwg4TUM3My7iv"));
    check_one(sp.get_track_audio_features("49A10weBfgwg4TUM3My7iv"));
    recommendations r;
    char* seed_artists[] = {"4NHQUGzhtTLFvgF5SZesLK"};
    r.seed_artists = seed_artists;
    r.seed_artists_size = 1;
    r.target_acousticness = 0.5;
    r.target_danceability = 0.5;
    check_one(sp.get_recommendations(r, 2));
    Serial.println("User");
    check_one(sp.get_current_user_profile());
    check_one(sp.get_user_profile("spotify"));
    check_one(sp.get_user_top_items(TOP_TYPE_ARTIST));
    check_one(sp.follow_playlist("37i9dQZF1EQn4jwNIohw50", false));
    check_one(sp.unfollow_playlist("37i9dQZF1EQn4jwNIohw50"));
    char* users[] = {"spotify"};
    check_one(sp.check_if_users_follow_playlist("37i9dQZF1EQn4jwNIohw50",1, users));
    check_one(sp.get_followed_artists(""));
    check_one(sp.follow_artists_or_users(FOLLOW_TYPE_USER, 1, users));
    check_one(sp.unfollow_artists_or_users(FOLLOW_TYPE_USER, 1, users));
    check_one(sp.check_if_user_follows_artists_or_users(FOLLOW_TYPE_USER, 1, users));*/
}
void check_one(response r){
    if(r.status_code < 200 or r.status_code > 300){
        Serial.printf("Error: %d\n", r.status_code);
        Serial.println(r.reply);
    }
}