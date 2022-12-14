/*************************************************************************************
remko2mqtt Interface for ESP8266
Destination-Hardware: NodeMCU ESP8266 with optional W5500 Ethernet-Shield
v1.3.0 (c) 2022 Dr.-Ing. Christian Nöding

I'm not related to Remko and this software is not an official part of Remko.
It is an attempt to improve the controllability of Remko devices for private use. Don't
bother Remko with problems that may occur by using this software. Don't use this software
at all if you do not understand parts of it or if you are unsure what this piece
of software is doing at all.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

Required Arduino-Libraries:
===========================
- Nick O'Leary, PubSubClient

Hardware-Connection:
====================
To connect to Remko, please add a level-shifter for each XT-output:
                      +5V
                       |
                      _|_
                     |   |
                     | 4 |
                     | k |
                     | 7 |
                     |___|
                       |
                       |-------- to Remko display-PCB, "XT"
                       |
                   |  /
    |---------|    | /
----|   10k   |----|/ BC337 or similar
    |---------|    |\
                   | \|
                     -|
                      |
                      |
                     GND

To receive data from Remko connect the RCV-pin via a levelshifter to the ESP8266

                  from Remko
               display-PCB, "RCV"
                       |
                      _|_
                     |   |
                     | 2 |
                     | k |
                     | 2 |
                     |___|
                       |
                       |-------- to ESP8266
                      _|_
                     | 6 |
                     | k |
                     |...|
                     | 8 |
                     |_k_|
                       |
                       |


At the moment a maximum of 4 devices can be controlled by this software, but could be
increased to more. The used GPIO-pins can be changed using the config.h

D0 GPIO16  -> W5500 RST (Reset)
D1 GPIO5   -> Remko XT0 (Device 0 TxD)
D2 GPIO4   -> Remko RCV0 (Device 0 RxD)
D3 GPIO0   -> Remko XT1 (Device 1 TxD)
D4 GPIO2   -> Remko RCV1 (Device 1 RxD)
D5 GPIO14  -> W5500 SCLK or Remko XT3 (Device 3 TxD)
D6 GPIO12  -> W5500 MISO or Remko RCV3 (Device 3 RxD)
D7 GPIO13  -> W5500 MOSI
D8 GPIO15  -> W5500 SCS (ChipSelect)
RxD GPIO3  -> Remko XT2 (Device 2 TxD)
TxD GPIO1  -> Remko RCV2 (Device 2 TxD)
GND
3V3

GPIOs 0, 2 and 15 control the boot of ESP8266:
0 2 15 Mode   Description
==================================================
L H H  Flash  Boot from SPI Flash (Normal running) 
L L H  UART   Program via UART (Tx/Rx)
H x x  SDIO   Boot from SD-Card

GPIO9 and GPIO10 are reserved for flash-chip but could be used as
GPIOs by reconfiguring the flash-communication

As the GPIO15 (SPI CS) is wired to ESP8266 Flash to boot you have to put a
4k7 resistor between GPIO15 and GND to let the ESP8266 boot correctly!


Additional links:
=================
The ESP8266 Arduino-Core has support for W5500- and other Ethernet-Chips since v3.0.2
Information about using ESP8266 with ethernet (W5500, W5100 or ENC28J20):
https://github.com/nopnop2002/esp8266_ethernet

General information and schematics
https://esp8266hints.wordpress.com/2018/02/13/adding-an-ethernet-port-to-your-esp-revisited/

GitHub Code
https://github.com/PuceBaboon/ESP-Now-Gateway

GitHub Code of the WIZ Ethernet Library for ESP8266
https://github.com/Wiznet/WIZ_Ethernet_Library

**************************************************************************************/

#include "Arduino.h"
#include <stdlib.h> // atoi()
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "config.h"
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

#ifndef UseWiFi
  #define W5500_RESET_Pin 16
  #define CSPIN 15
  Wiznet5500lwIP eth(CSPIN);
  //Wiznet5100lwIP eth(CSPIN);
  //ENC28J60lwIP eth(CSPIN);
#endif

// Setup the WiFi
#ifdef UseWiFi
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
  bool WiFiAvailable = false;
#endif

WiFiClient mqttnetworkclient; // MQTT-Client
ESP8266WebServer webserver(80); // HTTP-Server
ESP8266HTTPUpdateServer httpupdater; // WebUpdater

// MQTT definitions
PubSubClient mqttclient(mqttnetworkclient);

// define Tickers
Ticker TimerSeconds;
uint8_t TimerCounter = 0;

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
  TimerCounter++;
  if (TimerCounter>=mqtt_publish_interval) {
    TimerCounter=0;

    // send status via MQTT if connected
    if (mqttclient.connected()){
      remko_publishmqtt();
    }
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

  // initialize sending of Remko-commands
  remko_txd_init();
  #if RemkoRxDevices >= 1
    // initialize reading of Remko-commands
    remko_rxd_init();
  #endif

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
  // output cmd-bitstream if we have bits to send
  // remko_txd_step() is a time-critical function. So do not
  // put any delays within the loop(). Otherwise the individual
  // bits will have the wrong timing
  remko_txd_step();

  #if RemkoRxDevices >= 1
    remko_rxd_step();
  #endif
  
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
    remko_initmqtt();
  }else{
    // connected to MQTT-server
    mqttclient.loop(); // process incoming messages
  }
}

void MQTT_Callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // null-terminate byte-array

  // preload TimerCounter to 2s before interval so that new values are send right after the bitstream has finished
  TimerCounter=mqtt_publish_interval-2;

  remko_txd_processmqtt(topic, (uint8_t)String((char*)payload).toInt());
}
