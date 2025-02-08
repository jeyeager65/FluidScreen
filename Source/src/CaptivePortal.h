#ifndef CAPTIVEPORTAL_H
#define CAPTIVEPORTAL_H

#include <Arduino.h>
#include <Wifi.h>
#include <DNSServer.h>
#include "ESPAsyncWebServer.h"
#include <SPIFFS.h>

// Captive Portal
DNSServer dnsServer;
AsyncWebServer server(80);
const byte DNS_PORT = 53;
IPAddress apIP(4, 3, 2, 1);
const String urlIP = "http://4.3.2.1";

JsonObject saveJson;

void setupServer(){

  if(!SPIFFS.begin()) {
    Serial.println("Error mounting SPIFFS file system");
  }

  // Reference: https://github.com/CDFER/Captive-Portal-ESP32/blob/main/src/main.cpp

  // Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->redirect(urlIP); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->redirect(urlIP); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->redirect(urlIP); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->redirect(urlIP); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->redirect(urlIP); });			   // windows call home

  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) { Serial.println("Apple - hotspot detect"); request->redirect(urlIP); });
  server.on("/library/test/success.html", HTTP_GET, [](AsyncWebServerRequest *request) { Serial.println("Apple - library test"); request->redirect(urlIP); });
  
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) { Serial.println(request->url()); request->send(404); });	// webpage icon

  server.on("/current", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getJsonFromSettings(CurrentSettings);
    request->send(200, "application/json", json);
  });

  server.on("/default", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getJsonFromSettings(DefaultSettings);
    request->send(200, "application/json", json);
  });

  AsyncCallbackJsonWebHandler* handler = 
    new AsyncCallbackJsonWebHandler("/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
      Serial.println("Processing JSON Update");
      saveJson = json.as<JsonObject>();
      //performSave = true;
      FluidScreenSettings settings = getSettingsFromJson(saveJson);
      debugSettings(settings);
      saveSettings(settings, false);
      request->send(200, "text/html", "OK");
    });
  server.addHandler(handler);

  AsyncCallbackJsonWebHandler* previewHandler = 
    new AsyncCallbackJsonWebHandler("/preview", [](AsyncWebServerRequest *request, JsonVariant &json) {
      Serial.println("Previewing Theme");
      saveJson = json.as<JsonObject>();
      FluidScreenSettings settings = getSettingsFromJson(saveJson);
      debugSettings(settings);
      setupColors(settings);
      drawPreview();
      request->send(200, "text/html", "OK");
    });
  server.addHandler(previewHandler);
    
  server.on("/restart", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Processing Restart");
    setConfigMode(false);
    request->send(200);
    delay(5000);
    ESP.restart();
  });

  server.serveStatic("/css/", SPIFFS, "/www/css/", "no-cache");
  server.serveStatic("/js/", SPIFFS, "/www/js/", "no-cache");
  server.serveStatic("/", SPIFFS, "/www/", "no-cache").setDefaultFile("index.html");
  
  // Handle CORS
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.println("Not Found");
    Serial.println(request->url());
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      AsyncWebServerResponse *response = request->beginResponse(302);
      response->addHeader("Location", urlIP);
      request->send(response);
    }
  });

}

void captivePortalSetup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP("FluidScreen");

  if (dnsServer.start(DNS_PORT, "*", apIP)) {
    Serial.println("Started DNS server in captive portal-mode");
  } else {
    Serial.println("Err: Can't start DNS server!");
  }

  setupServer();

  server.begin();
}

void captivePortalLoop() {
  dnsServer.processNextRequest();
}

#endif