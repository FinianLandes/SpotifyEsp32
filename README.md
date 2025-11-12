# SpotifyESP32

This library is a wrapper for the [Spotify Web API](https://developer.spotify.com/documentation/web-api/) designed to work with the [ESP32](https://www.espressif.com/en/products/socs/esp32/overview) microcontroller.

⚠️ **Version 3 Notice:** This release may not be backward compatible with v2.x.x
The authetication process has changed to comply with the new spotify guidelines.

## Dependencies

- [ArduinoJson](https://arduinojson.org/)  
- [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/release/v2.x/libraries/WiFiClientSecureh) *(Note: In Arduino-ESP32 v3.x, `WiFiClientSecure` is a compatibility alias for `NetworkClientSecure`. This library uses `WiFiClientSecure` to ensure full compatibility with **PlatformIO**, where v3.x support is still unavailable.)*

## Setup

 **[YouTube Tutorial v3](https://youtu.be/Yy75KzIfqi4)**

### 1. Create a Spotify Application

1. Go to the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/applications).
2. Create a new application and copy your **Client ID** and **Client Secret**.
3. Add the following redirect URI: <https://spotifyesp32.vercel.app/api/spotify/callback>
4. Enable the **Web API** option.

### 2. Example: Login without a saved refresh token

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyEsp32.h"

const char* SSID = "your_ssid";
const char* PASSWORD = "your_password";
const char* CLIENT_ID = "your_client_id";
const char* CLIENT_SECRET = "your_client_secret";

// Create an instance of the Spotify class (optional: specify retry count)
Spotify sp(CLIENT_ID, CLIENT_SECRET);

void setup() {
 Serial.begin(115200);
 connect_to_wifi();

 // Optionally set custom scopes the available scopes are listed below
 // sp.set_scopes("user-read-playback-state user-modify-playback-state");

 sp.begin();
 while (!sp.is_auth()) {
     sp.handle_client(); // Required for receiving the authorization code
 }

 Serial.printf("Authenticated! Refresh token: %s\n", sp.get_user_tokens().refresh_token);
}

void loop() {
 // Your code here
}

void connect_to_wifi() {
 WiFi.begin(SSID, PASSWORD);
 Serial.print("Connecting to WiFi...");
 while (WiFi.status() != WL_CONNECTED) {
     delay(1000);
     Serial.print(".");
 }
 Serial.println("\nConnected to WiFi!");
}
```

[List of available scopes](https://developer.spotify.com/documentation/web-api/concepts/scopes)

### 3. Save the Refresh Token

After logging in via the URL shown in the Serial Monitor, your ESP32 will print a refresh token.
Copy this token and pass it as the third parameter to the constructor.

```cpp
Spotify sp(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);
```

This way, you won’t have to reauthenticate each time.

### 4. Setting Tokens at Runtime

If you prefer setting tokens during runtime (for example, using a web server), you can:
Pass an empty string for the refresh token during initialization.
Later, call get_user_tokens() to retrieve the tokens.
Store them in flash memory (e.g., using [SPIFFS](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/spiffs.html))

### 5. Using the Library

Each API call returns a response object containing:
`response_obj.status_code` → the HTTP status code (or -1 if the request failed before sending)
`response_obj.reply` → the JSON response as a JsonDocument
To print a response: `print_response(response_obj);`
To reduce memory usage, GET requests support **filtered responses** ([Filter tutorial](https://arduinojson.org/news/2020/03/22/version-6-15-0/)):

```cpp
JsonDocument filter;
filter["item"]["name"] = true;
response res = sp.get_current_playback(filter);
```

See the [Spotify Web API Reference](https://developer.spotify.com/documentation/web-api/reference/) for all the possible endpoints.

## Optimization Options

To reduce flash usage, disable unneeded endpoints by defining macros before including the library:

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
```

## Helper Functions

```c++
// Current playback info
String current_track_name();
String current_track_id();
String current_device_id();
String current_artist_names();

// Versions returning pointers (e.g for as parameters for other functions)
char* current_device_id(char* device_id);
char* current_track_id(char* track_id);
char* current_track_name(char* track_name);
char* current_artist_names(char* artist_names);

// Playback and device info
bool is_playing();
bool volume_modifyable();

// URI helpers
char  convert_id_to_uri(char* id, char* type);
char* convert_id_to_uri(char* id, char* type, char* uri);

// Current album artwork url
String get_current_album_image_url(int image_size_idx);
```

You can also include the namespace:

```cpp
using namespace spotify_types;
```

## Token Management

- Retrieve stored tokens (useful for saving to flash memory):

    ```cpp
    user_tokens tokens = sp.get_user_tokens();
    ```

    This contains `client_id`, `client_secret`, and `refresh_token`.

- The library automatically refreshes expired access tokens before making API requests.
You can also refresh manually:

    ```cpp
    sp.get_token();
    ```

    Returns `true` if successfull.

## Debugging

The SpotifyEsp32 library features a custom logging system, independent of `esp_log.h`, to assist with effective issue diagnosis. You can configure the logging level using the following method:

```cpp
sp.set_log_level(spotify_log_level_t spotify_log_level);
```

### Available Logging Levels

The library provides the following logging options to control output verbosity:

- `SPOTIFY_LOG_NONE`: Disables all logging output.
- `SPOTIFY_LOG_ERROR`: Captures both fatal and non-fatal errors for critical issues.
- `SPOTIFY_LOG_WARN`: Includes warnings alongside error messages.
- `SPOTIFY_LOG_INFO`: Offers additional informational messages about general operation.
- `SPOTIFY_LOG_DEBUG`: Provides detailed debug information for troubleshooting.
- `SPOTIFY_LOG_VERBOSE`: Delivers the highest level of detail with extensive logging.

### Default Setting

The default logging level is `SPOTIFY_LOG_NONE`, meaning no logs are generated unless explicitly enabled.

## Memory and Flash usage

Because this library uses WiFi and HTTPS, it requires more flash memory than a typical ESP32 sketch.
You may need to increase the application partition size (above the default 1.2 MB).

- [Official Espressif Documentation](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html?highlight=partitions)
- [Partition Table in the Arduino IDE](https://robotzero.one/arduino-ide-partitions/)
- [Partition Table in Platform IO](https://docs.platformio.org/en/latest/platforms/espressif32.html)

If flash space is limited, disable unused endpoints.

## Troubleshooting

- Enable debug mode by passing `true` as second last argument to the constructor.
- If requests fail, inspect the returned response or Serial output.
- Test individual endpoints in the [Spotify Web API Console](https://developer.spotify.com/console/). </br>
- Still having issues? Open an issue in this repository.

## Supported Devices

- ESP32 WROOM
- Should also work on other ESP32 models.
- For now it probably uses too much flash and memory to run on a standard ESP2866
