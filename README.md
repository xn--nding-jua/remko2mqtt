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

# First progress
Remko seems to use a very easy protocol for their IR-remote controls and use this IR code for the interface-PCB, too. First measurements show, that the interface-PCB uses a Sino Wealth SH79F161F 8051 microcontroller, that simply emulates the IR-remote-control, when it detects a change on SWITCH1/2 or ON/OFF inputs. As the RS485 IC for the XYE-connector (CN403) is connected to the microcontroller unidirectional, I guess Remko uses the same IR-protocol for their MCC-1 or SC-1 controllers, too. If this is true, it will be harder to get to the room-temperature as this seems to be transmitted via RF.

![image](https://user-images.githubusercontent.com/9845353/200952754-aaace814-bc4a-4f64-8142-94f9f48cabbc.png)

With my first measurements I got some information about the communication between the Display-PCB and the interface-PCB:
- On connector CN501 (GND, 5V, XT, REC, 5V) the interface-PCB transmit the commands of the IR-remote-control on pin XT
- standby-voltage is +5V and individual bits toggle to 0V (so we have TTL-level)
- one command is send two times
- each bit takes 550µs, so we have a 1800 baud communication
- communication starts with 8 bits (4.4ms) at low level (GND), followed by 8 bits (4.4ms) at high-level (5V)
- afterwards a modulation 
- communication ends on high-level (5V)

![image](https://user-images.githubusercontent.com/9845353/200953658-b8c471ee-0560-470b-b5ee-80029d5ff555.png)

In the next step I will dive into the protocol and try to generate these signals with the ESP8266. It seems, that it is not a real communication-protocol, but only a predefined bit-toggle. I figured out, that each temperature-value the remote-command sends is stored as an individual bit-pattern - so each function of the Remko system seems to be coupled to an individual pattern. With this knowledge it should be possible to create an easy ESP8266-dongle that controls each MXW-device.
