# remko2mqtt
Control your Remko Air Conditioner via MQTT using an ESP8266

The goal of this project is to control a Remko air conditioner (mainly the MXW, but maybe the SKW, RVT, ATY, ML, RVD or RXT, too) via MQTT and NodeRED without using the Remko Smart-Control WiFi-Stick and an additional mobile-phone app. As the Stick only supports devices since 08/2021 I was not able to use this option anyway. These devices are tested at the moment:
- [x] MXW353 (manufactured in 2016)

Current state of this project:
- [x] Investigation of the original interface-PCB using the switchboard with the original 5-pin-cable (+5V, GND, XT and RCV) together with IR-controller
- [x] Write code to emulate the necessary commands with 1800 Baud
- [ ] Create an adapter-board using ESP8266 to emulate original interface-PCB (mainly a simple level-shift from 3V3 to 5V)
- [ ] Enhance code to support more than one device addressed via MQTT-topic using multiple GPIOs
- [ ] Reverse engineering of the communication-protocol of an original Remko Smart-Control WiFi-Stick to find more commands
- [ ] Enhance the written ESP8266-program with discovered commands of original WiFi-Stick

At the moment the following commands are supported:
* set device on/off
* set operation mode (heating or cooling)
* set desired temperature
* toggle led

Nice-to-have feature will be:
* bidirectional communication to readout the current temperature
* Follow-Me-Function for remote-temperature
* read other parameters of device

Start of project: 09.11.2022

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

I figured out, that Remko does not use a "real" communication-protocol, but only a predefined bit-toggle for each function. So each temperature-value of the remote-control has its individual bit-pattern. I've implemented an Arduino-Test-Sketch to send this bit-pattern via a GPIO. I've already implemented the most-important functions (on/off, temperature-setpoint, operation-mode, etc.). Now I have to test the functions on the real device. If everything is working multiple devices can be controlled using one single ESP as we are only limited by the available GPIOs as we do not need a full UART.
