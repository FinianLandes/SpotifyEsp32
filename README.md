# Spotify Library for ESP32 
This library is a wrapper for the [Spotify Web API](https://developer.spotify.com/documentation/web-api/) and is designed to work with the [ESP32](https://www.espressif.com/en/products/socs/esp32/overview) microcontroller. 

## Dependencies
- [ArduinoJson](https://arduinojson.org/) </br>
- [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/NetworkClientSecure) </br>
- [base64](https://github.com/Densaugeo/base64_arduino) </br>
- [WebServer](https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/src/WebServer.h) (Optional) </br>

## Setup
[Youtube Tutorial for the setup](https://www.youtube.com/watch?v=xNjbRq59dlc)</br>
1. Create a new application on the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/applications) and copy the Client ID and Client Secret into your code. Leave the developer dashboard open as you will need to set the callback url later. </br>
2. Now you will have to use the login without a refresh token which can be implemented the following way:
```c++
#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyEsp32.h"

const char* SSID = "your_ssid";
const char* PASSWORD = "your_password";
const char* CLIENT_ID = "your_client_id";
const char* CLIENT_SECRET = "your_client_secret";

//Create an instance of the Spotify class Optional: you can set the Port for the webserver the debug mode(This prints out data to the serial monitor) and number of retries
Spotify sp(CLIENT_ID, CLIENT_SECRET);

void setup() {
    Serial.begin(115200);
    connect_to_wifi();//Connect to your wifi
    
    sp.begin();//Start the webserver
    while(!sp.is_auth()){//Wait for the user to authenticate
        sp.handle_client();//Handle the client, this is necessary otherwise the webserver won't work
    }
    Serial.println("Authenticated");
}

void loop() {
    //Add your code here
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
```
3. Now you will have to set the callback url in the Spotify Developer Dashboard before you can log in the callback url is the url printed to the serial plus callback so e.g. if your url printed to the serial is: ```https://192.1.1.128/``` then the callback url will be ```https://192.1.1.128/callback```  </br>

4. If you want to set your tokens during runtime you can use the same code pass an empty char array to the constructor and set the tokens later in the webserver. By calling the "get_tokens" function you could then save these tokens with [SPIFFS](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/spiffs.html). And during next runtime pass them from the memory. </br>
5. Now you can use the library. </br>
## Usage
- The normal functions return an response object. You can get the http code or incase the token request failed and the request wasn't sent -1 with ```response_obj.status_code```. The response message can be obtained as JsonDocument with ```response_obj.reply```. </br>
To print the response you can use the ```print_response(response_obj)``` function. </br>
- To minimize RAM usage all functions making a GET request have an optional last parameter which is a filter JsonDocument which can be used to filter the response and only get the necessary data. Filtering works the following way: you pass a JsonDocument with the same structure than the response (This structure can be looked up in the Documentation of the Spotify Web API) where wanted data is just set to true and unwanted data is not included. Sadly there is no new tutorial for v7 of Arduino Json but the one of v6 is nearly identical except the definition of the JsonDocumet which can now be done in all cases like that: ```JsonDocument doc;``` [Filter tutorial](https://arduinojson.org/news/2020/03/22/version-6-15-0/)  </br>
- To search for methods you can use the [Spotify Web API Reference](https://developer.spotify.com/documentation/web-api/reference/), all of the methods shown there are implemented </br>
- To minimize FLASH usage you can add these Macros before including the library, these can disable the individual endpoints and the webserver functionality. </br>
```c++
#define DISABLE_PLAYER
#define DISABLE_ALBUM
#define DISABLE_ARTIST
#define DISABLE_AUDIOBOOKS
#define DISABLE_CATEGORIES
#define DISABLE_CHAPTERS
#define DISABLE_EPISODES
#define DISABLE_GENRES
#define DISABLE_MARKETS
#define DISABLE_PLAYLISTS
#define DISABLE_SEARCH
#define DISABLE_SHOWS
#define DISABLE_TRACKS
#define DISABLE_USER
#define DISABLE_SIMPLIFIED
#define DISABLE_WEB_SERVER
```
- There are also some helper and some simplified functions. </br>
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
    //get current album url
    string get_current_album_image_url(int image_int);
```
- You can also include the namespace: ```using namespace Spotify_types;```. These are some types used in the library eg. TYPES, SIZES of uris/ids </br>
### Other Functions
- If you want to get the tokens which you set during runtime to save them and reuse them you can call the ```get_user_tokens()``` function, this function returns a ```user_tokens``` object which contains: ```client_id```,```client_secret``` and the ```refresh_token```. </br>
- To make a request the library automatically gets a new access token if there is no access token or the access token is expired. So if you want to already do that before making a request, e.g. in ```Setup()``` you can do that by calling sp.```get_token()``` this will return true if the token was obtained successfully. To check whether there is already a access token you can call ```has_access_token()```. But as said this is Optional, the library does that on its own.</br>
- If you want to use a different webserver library to obtain the refresh token you can disable the webserver functionality and the use ```get_refresh_token(const char* auth_code,const char* redirect uri)``` to get the refresh token by passing your obtained auth code and redirect uri. </br>
## Useful Information
Due to the Wifi and HTTPS this library is quite big, so i recommend setting the partitions table of your esp to use more than the default 1.2MB of flash as most esp32's have 4MB of which the biggest part is set for SPIFFS which in most cases isn't used. Here are some tutorials about setting the partitions table of your esp: </br>
- [Official Espressif Documentation](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html?highlight=partitions)</br>
- [Partition Table in the Arduino IDE](https://robotzero.one/arduino-ide-partitions/)</br>
- [Partition Table in Platform IO](https://docs.platformio.org/en/latest/platforms/espressif32.html)</br>
If you don't want or can do that you can also disable some endpoints and the webserver functionality to reduce the size of the library. </br>
## Trouble Shooting
- If you have any problems with the library you can use the debug mode to print out the data to the serial monitor. This can be done by passing true to the constructor of the library as additional parameter.</br>
- If requests fail there will be some information in the response message. </br>
- If you have any problems with the library you can also use the [Spotify Web API Console](https://developer.spotify.com/console/) to test the endpoints. </br>
- If there are still issues you can open an issue on this repository. </br>
## Working Devices
- ESP32 WROOM</br>
- Should also work on other ESP32 models.</br>
- For now it probably uses too much flash and memory to run on a standard ESP2866
