
// general software config
const char* versionstring = "v0.1.0 built on ";
const char compile_date[] = __DATE__ " " __TIME__;

#define InvertCommunication 1

#define UseWiFi 1

#ifdef UseWiFi
  // we are using WiFi connection
  const char* ssid = "MyVeryPrivateWiFiNetwork";
  const char* password = "MyVeryPrivateWiFiNetworkPassword";
#else
  // we are using ethernet-port via W5500

  // Enter a MAC address for your controller below.
  // Newer Ethernet shields have a MAC address printed on a sticker on the shield
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //MAC
#endif

const char* host = "remko2mqtt";

#define UseStaticIP 1
#ifdef UseStaticIP
  IPAddress local_IP(192, 168, 0, 42);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 0, 1);   //optional
  IPAddress secondaryDNS(1, 1, 1, 1); //optional
#endif

// setup MQTT
#define mqtt_id "remko2mqtt"
#define mqtt_server "192.168.0.41"
#define mqtt_serverport 1883
#define mqtt_topic_powerstate "remko/powerstate"
#define mqtt_topic_opmode     "remko/opmode"
#define mqtt_topic_setpoint   "remko/setpoint"
#define mqtt_topic_followme   "remko/followme"
#define mqtt_topic_led        "remko/led"
