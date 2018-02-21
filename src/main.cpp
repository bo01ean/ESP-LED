#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <SocketIoClient.h>


#include "EEPROMAnything.h"
#include "elapsedMillis.h"

#define USE_SERIAL Serial
#define LED_RED     15
#define LED_GREEN   12
#define LED_BLUE    13

const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;
ESP8266WebServer server(80);

void event(const char * payload, size_t length) {
  USE_SERIAL.printf("got message: %s\n", payload);
}

void setup() {
    USE_SERIAL.begin(115200);

    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

   if(MDNS.begin("esp8266")) {
       USE_SERIAL.println("MDNS responder started");
   }

    webSocket.on("event", event);
    webSocket.begin("esp8266.ws");

    server.on("/", []() {
       // send index.html
       server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/></body></html>");
    });

    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_BLUE, 0);

}

void loop() {
  webSocket.loop();
  server.handleClient();
}
