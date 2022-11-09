# remko2mqtt
Control your Remko Air Conditioner via MQTT using an ESP8266

The goal of this project is to control a Remko air conditioner (mainly the MXW-devices) via a cheap ESP8266-interface with MQTT and NodeRED without using the Remko Smart-Control WiFi-Stick. For this, several steps have been planned during the next few weeks:

- [ ] Reverse engineering of the original interface-PCB using the switchboard with the original 5-pin-cable (+5V, GND, XT and RCV)
- [ ] Create an adapter-board using ESP8266 to emulate original interface-PCB to turn-on or off the device using MQTT
- [ ] Reverse engineering of the communication-protocol of an original Remko Smart-Control WiFi-Stick
- [ ] Enhance the written ESP8266-program with discovered commands of original WiFi-Stick

At the end at least the following commands should be supported:
* set device on/off
* set operation mode (heating or cooling)
* set desired temperature

Nice-to-have feature will be:
* bidirectional communication to readout the current temperature
* read other parameters

Start of project: 09.11.2022. Please feel free to support this project, but do not expect working code at this stage as this project is in an early stage.
