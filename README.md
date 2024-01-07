# Spotify Library for ESP32 
This library is a wrapper for the [Spotify Web API](https://developer.spotify.com/documentation/web-api/) and is designed to work with the [ESP32](https://www.espressif.com/en/products/socs/esp32/overview) microcontroller. 
## Dependencies
- [ArduinoJson](https://arduinojson.org/) </br>
- [HTTPClient](https://github.com/amcewen/HttpClient) </br>
- [WiFi](https://www.arduino.cc/en/Reference/WiFi) </br>
- [UrlEncode](https://github.com/plageoj/urlencode) </br>

## Setup
1. Create a new application on the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/applications) and copy the Client ID and Client Secret. </br>
2. Get a refresh token with the needed scopes. (You can do that with this [tool](https://spotify-refresh-token-generator.netlify.app/#welcome)) </br>
3. Save refresh token, client id, client secret and redirect uri in your main.cpp or project_name.ino file. </br>
4. Create an instance of the Spotify class: </br>
```c++
Spotify sp("refresh_token", "client_id", "client_secret", "redirect_uri", "bool debug", "int number of retries"); 
```
Debug and number of retries are optional. Debug is false as default and retries is set to 3. </br>
5. Connect to Wifi: </br>
```c++
void connectToWifi(){
  WiFi.begin("your_ssid", "your_password");§
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}
```
This is a simple example using the WiFi library. </br>
6. Now you can call your needed functions on the Spotify instance. The functions have doxygen comments, so you can read them in your IDE. The functions are also corresponding to the [Spotify Web API](https://developer.spotify.com/documentation/web-api/). </br>
7. The normal functions return an response object. You can get the http code of type int with ```response_obj.status_code``` and the response message of type String with ```response_obj.reply```. </br>
To print the response you can use the ```print_response(response_obj)``` function. </br>
8. There are also some helper and some simplified functions. </br>
```c++
    // Get the current track name and return it as String
    String current_track_name();
    // Get the current track id and return it as String
    String current_track_id();
    // Get the current device id and return it as String
    String current_device_id();
    // Get the current artist names and return it as String
    String current_artist_names();
    // Get the current device id and return a pointer to the char, can be used as parameter for other functions
    char* current_device_id(char* device_id);
    // Get the current track id and return a pointer to the char, can be used as parameter for other functions
    char* current_track_id(char* track_id);
    // Get the current track name and return a pointer to the char, can be used as parameter for other functions
    char* current_track_name(char* track_name);
    // Get the current artist names and return a pointer to the char, can be used as parameter for other functions
    char* current_artist_names(char* artist_names);
    //Check if the device is playing and return true or false
    bool is_playing();
    //Check if volume of the current device is modifyable and return true or false
    bool volume_modifyable();
    //Convert a context id to a uri and return it as a char
    char convert_id_to_uri(char* id, char* type);
    //Convert a context id to a uri and return it as a pointer to the char
    char* convert_id_to_uri(char* id, char* type, char* uri); 
```
9. You can also include the namespace: ```using namespace Spotify_types;```. These are some types used in the library eg. TYPES, SIZES of uris/ids </br>
