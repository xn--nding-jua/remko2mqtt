/*************************************************************************************
remko2mqtt Interface for ESP8266
v0.0.1 (c) 2022 Dr.-Ing. Christian Nöding
Destination-Hardware: NodeMCU ESP8266 with optional W5500 Ethernet-Shield

I'm not related to Remko and this software is not an official part of Remko.
It is an attempt to improve the controllability of Remko internal parts. Don't
bother Remko with problems that occur by using this software. Don't use this software
at all if you do not understand parts of it or if you are unsure what this piece
of software is doing at all.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

**************************************************************************************/

#include "Arduino.h"
#include <stdlib.h> // atoi()
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#ifndef UseWiFi
  // use ethernet
  #include <SPI.h>
  #include <W5500lwIP.h>
  //#include <W5100lwIP.h>
  //#include <ENC28J60lwIP.h>
#endif
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>

// general things
const char* versionstring = "v0.0.1 built on ";
const char compile_date[] = __DATE__ " " __TIME__;

#ifndef UseWiFi
  #define W5500_RESET_Pin 16
  #define CSPIN 15
  Wiznet5500lwIP eth(CSPIN);
  //Wiznet5100lwIP eth(CSPIN);
  //ENC28J60lwIP eth(CSPIN);
#endif

// Setup the WiFi
#ifdef UseWiFi
  const char* ssid = "MyVeryPrivateWiFiNetwork";
  const char* password = "MyVeryPrivateWiFiNetworkPassword";
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
  bool WiFiAvailable = false;
#endif

#define UseStaticIP 1
const char* host = "remko2mqtt";
IPAddress local_IP(192, 168, 0, 42);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 0, 1);   //optional
IPAddress secondaryDNS(1, 1, 1, 1); //optional

WiFiClient mqttnetworkclient; // MQTT-Client
ESP8266WebServer webserver(80); // HTTP-Server
ESP8266HTTPUpdateServer httpupdater; // WebUpdater

// MQTT definitions
#define mqtt_id "remko2mqtt"
#define mqtt_server "192.168.0.41"
#define mqtt_serverport 1883
const char *mqtt_mppt_topic = "remko/mxw/";
PubSubClient mqttclient(mqttnetworkclient);

// do a minimal webserver to give a sign of live :-)
void handleRoot() {
  String message = "<html><font face=Arial size=4>Remko 2 MQTT Bridge<br></font>";
  message += "<input type=\"button\" onclick=\"location.href='update';\" value=\"Firmwareupdate\" /><br><br>";
  message += "<font face=Arial size=2>";
  message += versionstring;
  message += compile_date;
  message += "</font></html>";
  webserver.send(200, "text/html", message);
}

void handleNotFound() {
  String message = "File Not Found";
  webserver.send(404, "text/plain", message);
}

void TimerSecondsFcn() {
  if (mqttclient.connected()){
    // do something here every second
  }
}

#ifdef UseWiFi
  void init_WiFi() {
    // Start WiFi as station (client)
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    #ifdef UseStaticIP
      WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    #endif
    WiFi.begin(ssid, password);
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
  }
#else
  void init_eth() {
    // disable WiFi to save energy
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
  
    // Starting Wiz W5500 Ethernet via Reset-Pin
    pinMode(W5500_RESET_Pin, OUTPUT);
    digitalWrite(W5500_RESET_Pin, HIGH);
    delay(250);
    digitalWrite(W5500_RESET_Pin, LOW);
    delay(50);
    digitalWrite(W5500_RESET_Pin, HIGH);
    delay(350);
    
    // start ethernet
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(20000000); // max. 33MHz
    
    eth.setDefault(); // use ethernet for default route
    #ifdef UseStaticIP
      eth.config(local_IP, gateway, subnet);
    #endif
    pinMode(15, OUTPUT);
    digitalWrite(15, LOW); // set SPI CS to low to safe energy as we are using a pulldown against GND -> only set to HIGH when needed

    int present = eth.begin(mac);
    if (!present) {
      //Serial.println("no ethernet hardware present");
      while(1);
    }
    while (!eth.connected()) {
      //Serial.print(".");
      delay(500);
    }
    /*
    Serial.print("ethernet ip address: ");
    Serial.println(eth.localIP());
    Serial.print("ethernet subnetMask: ");
    Serial.println(eth.subnetMask());
    Serial.print("ethernet gateway: ");
    Serial.println(eth.gatewayIP());
    */
  }
#endif

void setup() {
  delay(1000);

  #ifndef UseWiFi
    init_eth();
  #endif

  // Start the HTTP-Server
  // starting ESP8266-HTTP-UpdateServer
  httpupdater.setup(&webserver);
  webserver.on("/", handleRoot);
  webserver.onNotFound(handleNotFound);
  webserver.begin();

  // publish DNS-name
  MDNS.begin(host);
  MDNS.addService("http", "tcp", 80);

  // configure MQTT-client
  mqttclient.setServer(mqtt_server, mqtt_serverport);
  mqttclient.setCallback(MQTT_Callback);

  // Initiate the timers
  TimerSeconds.attach_ms(1000, TimerSecondsFcn);

  #ifdef UseWiFi
    gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)
    {
      //Serial.print("Station connected, IP: ");
      //Serial.println(WiFi.localIP());
  	  WiFiAvailable = true;
    });
    
    disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
    {
      //Serial.println("Station disconnected");
  	  WiFiAvailable = false;
    });
  #endif
}

void loop() {
  #ifdef UseWiFi
  if (WiFiAvailable) {
  #else
  if (eth.connected()) {
  #endif
    // network is connected... do something

    // handle MQTT-transmission
    HandleMQTT();

    // handle webserver on port 80
    webserver.handleClient();

    // update DNS-configuration
    MDNS.update();
  #ifdef UseWiFi
  }else{
    init_WiFi(); // (re)connect to WiFi
  #endif
  }
}
  
void HandleMQTT() {
  if (!mqttclient.connected()) {
    // not connected so try to connect
    mqttclient.connect(mqtt_id);

    // subscribe to the desired topics
	  mqttclient.subscribe("remko/mxw/set/powerstate");
	  mqttclient.subscribe("remko/mxw/set/opmode");
	  mqttclient.subscribe("remko/mxw/set/setpoint");
  }else{
    // connected to MQTT-server
    mqttclient.loop(); // process incoming messages
  }
}

void PublishMQTT(char* pretopic, char* topic, float payload) {
  char mqtt_payload[10];
  dtostrf(payload, 2, 4, mqtt_payload);

  char mqtt_topic[40];
  strcpy(mqtt_topic, pretopic);
  strcat(mqtt_topic, topic);
  mqttclient.publish(mqtt_topic, mqtt_payload);
}

void MQTT_Callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // null-terminate byte-array

  if (strcmp(topic, "remko/mxw/set/powerstate")==0) {
    // set powerstate
  }
  
  if (strcmp(topic, "remko/mxw/set/opmode")==0) {
    // set powerstate
  }
  
  if (strcmp(topic, "remko/mxw/set/setpoint")==0) {
    // set powerstate
  }
}
