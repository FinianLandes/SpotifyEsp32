#include <Arduino.h>
#include <EspAsyncWebServer.h>
#include "index.h"
const char* ssid = "Rebweg10D";
const char* password = "Bitte_eintreten";
char* client_id = "51dbbb193b9a4ad09225a61cf4cf08a0";
char* client_secret = "92c2c4d564174b4f851a813fbc79715c";
char redirect_uri[400] = "";
int port = 8000;
char refresh_token[300] = "";

void connect_to_wifi();
void get_refresh_token();
void server_on_callback(AsyncWebServerRequest *request);
AsyncWebServer server(8000);
void setup() {
  Serial.begin(115200);
  connect_to_wifi();
  get_refresh_token();
  while (strcmp(refresh_token, "") == 0) {
    delay(1000);
  }
  Serial.print("Refresh token: ");
  server.end();
}

void loop() {
  
}

void get_refresh_token(){
  sprintf(redirect_uri, "http://%s:%d/callback", WiFi.localIP().toString().c_str(), port);
  Serial.print("Go to this url to login to spotify: ");
  Serial.println(redirect_uri);
  server.on("/callback", HTTP_GET, server_on_callback);
  server.begin();
}

void server_on_callback(AsyncWebServerRequest *request) {
    if (strcmp(refresh_token, "") == 0) { 
        if (request->args() == 0) {
            char page[900];
            sprintf(page, login_page, client_id, redirect_uri);
            request->send(200, "text/html", String(page));
        } else {
            if (request->hasArg("code")) {
                request->send(200, "text/html", "Setup Complete <br>You can now close this page");
                strcpy(refresh_token, request->arg("code").c_str()); 
            } else {
                char page[900];
                sprintf(page, login_page, client_id, redirect_uri);
                request->send(200, "text/html", String(page)); 
            }
        }
    } else {
        request->send(200, "text/html", "Spotify setup complete");
    }
}

void connect_to_wifi(){
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}

