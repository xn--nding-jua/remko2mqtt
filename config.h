// general software config
const char* versionstring = "v1.3.0 built on ";
const char compile_date[] = __DATE__ " " __TIME__;
const char* host = "remko2mqtt";

#define UseWiFi 1
#define UseStaticIP 1
#define MirrorRxD2TxD 1 // mirrors the received commands to the defined output-pin

// select how many Remko-devices you want to control and on which GPIO
#define RemkoTxDevices 2 // define the numbers of devices you want to control (1...4) - only 3 Devices possible if using W5500 ethernet
#define RemkoRxDevices 2 // define the numbers of devices you want to read (0...4) - only 3 Devices possible if using W5500 ethernet
uint8_t RemkoTxPin[] = {5, 0, 3, 14}; // Enter the desired GPIO pins in this array
uint8_t RemkoRxPin[] = {4, 2, 1, 12}; // Enter the desired GPIO pins in this array
#define RemkoInvertTxD 1 // invert Tx-bits (e.g. when using a transistor with pullup-resistor)
//#define RemkoInvertRxD 1 // invert Rx-bits

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

#ifdef UseStaticIP
  IPAddress local_IP(192, 168, 0, 42);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 0, 1); //optional
  IPAddress secondaryDNS(1, 1, 1, 1); //optional
#endif

// setup MQTT-server
#define mqtt_id "remko2mqtt"
#define mqtt_server "192.168.0.41"
#define mqtt_serverport 1883
#define mqtt_publish_interval 60

// MQTT-topics for management and debugging
#define mqtt_topic_restart             "remko/cmd/restart"
#define mqtt_topic_set_debuglevel      "remko/set/debuglevel"
#define mqtt_topic_debug               "remko/debug"

// MQTT-topics for subscribing
#define mqtt_topic_dev0_set_poweroff   "remko/0/set/poweroff"
#define mqtt_topic_dev0_set_opmode     "remko/0/set/opmode"
#define mqtt_topic_dev0_set_setpoint   "remko/0/set/setpoint"
#define mqtt_topic_dev1_set_poweroff   "remko/1/set/poweroff"
#define mqtt_topic_dev1_set_opmode     "remko/1/set/opmode"
#define mqtt_topic_dev1_set_setpoint   "remko/1/set/setpoint"
#define mqtt_topic_dev2_set_poweroff   "remko/2/set/poweroff"
#define mqtt_topic_dev2_set_opmode     "remko/2/set/opmode"
#define mqtt_topic_dev2_set_setpoint   "remko/2/set/setpoint"
#define mqtt_topic_dev3_set_poweroff   "remko/3/set/poweroff"
#define mqtt_topic_dev3_set_opmode     "remko/3/set/opmode"
#define mqtt_topic_dev3_set_setpoint   "remko/3/set/setpoint"

// MQTT-topics for publishing
#define mqtt_topic_dev0_powerstate      "remko/0/powerstate"
#define mqtt_topic_dev0_opmode          "remko/0/opmode"
#define mqtt_topic_dev0_setpoint        "remko/0/setpoint"
#define mqtt_topic_dev1_powerstate      "remko/1/powerstate"
#define mqtt_topic_dev1_opmode          "remko/1/opmode"
#define mqtt_topic_dev1_setpoint        "remko/1/setpoint"
#define mqtt_topic_dev2_powerstate      "remko/2/powerstate"
#define mqtt_topic_dev2_opmode          "remko/2/opmode"
#define mqtt_topic_dev2_setpoint        "remko/2/setpoint"
#define mqtt_topic_dev3_powerstate      "remko/3/powerstate"
#define mqtt_topic_dev3_opmode          "remko/3/opmode"
#define mqtt_topic_dev3_setpoint        "remko/3/setpoint"
