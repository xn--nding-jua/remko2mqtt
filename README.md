# remko2mqtt
Control your Remko Air Conditioner via MQTT using an ESP8266

# Content
- [Introduction](#introduction)
- [Using the software](#software)
- [Information about the used communication-protocol](#information)
- [Outlook](#outlook)

<a name="introduction"></a>
# Introduction
With this Arduino-project you can control your Remko air conditioner (mainly the MXW, but maybe the SKW, RVT, ATY, ML, RVD or RXT, too) via MQTT and - for example - NodeRED. As the Remko Smart-Control WiFi-Stick only supports devices since 08/2021 I was not able to use this option for my devices, so I programmed this piece of software. At the moment the following device is tested:
- [x] MXW353 (manufactured in 2016)

## Current state of this project:
- [x] Investigation of the original interface-PCB using the switchboard with the original 5-pin-cable (+5V, GND, XT and RCV) together with IR-controller
- [x] Write code to emulate the necessary commands with 1800 Baud
- [x] Create an adapter-board using ESP8266 to emulate original interface-PCB (mainly a simple level-shift from 3V3 to 5V)
- [x] Enhance code to support more than one device addressed via MQTT-topic using multiple GPIOs
- [ ] Discover the used protocol for XYE-connection to use RS485 with address-setting for supporting multiple devices
- [ ] Reverse engineering of the communication-protocol of an original Remko Smart-Control WiFi-Stick to find more commands
- [ ] Enhance the written ESP8266-program with discovered commands of original WiFi-Stick

At the moment the following commands are supported via direct connection of ESP8266 to display-PCB (without using interface-PCB):
- [x] set device on/off
- [x] set operation mode (heating or cooling)
- [x] set desired temperature
- [x] toggle led

Future "nice-to-have" feature will be:
- [ ] bidirectional communication to readout the current temperature
- [ ] Follow-Me-Function for remote-temperature
- [ ] read other parameters of device

<a name="software"></a>
# Using the software
To use this software with your own Remko-devices you have to compile the software using Arduino with installed ESP8266-support and PubSubClient-library. In the config.h you have to configure the software (IP-Addresses, MQTT-Server, and so on). You can choose between using WiFi or an ethernet-connection using the W5500-ethernet-shield. Furthermore you can select how many Remko-devices you'd like to use (at the moment up to four) and on which GPIO you want to output the commands. The IP-addresses and MQTT-topics can be set within the config.h, too. Finally, I've implemented an OTA-firmware-update-feature that allows you to update the firmware via the webpage. Just open the IP-address of the device with a browser, click on the button "Firmwareupgrade" and follow the instructions. A new firmware-binary can be created using Arduino using "Ctrl+Alt+S".

The ESP8266 can be connected via a simple level-shifter-circuit using a BC337 transistor (or similar):
![image](https://user-images.githubusercontent.com/9845353/201294124-841eece7-cbb7-40e3-a969-25299c0f4f51.png)

<a name="information"></a>
# Information about the used communication-protocol
Remko uses predefined commands/bit-pattern for each function of their IR-remote controls and uses these static commands/patterns for their interface-PCB, too. The Sino Wealth SH79F161F 8051 microcontroller on the interface-PCB sends these commands, when it detects a change on SWITCH1/2 or ON/OFF inputs, for example.

So each temperature-value of the remote-control has its individual bit-pattern. I've implemented an Arduino-Test-Sketch to send this bit-pattern via a GPIO. I've already implemented the most-important functions (on/off, temperature-setpoint, operation-mode, etc.):
```C
// TurnOn
{0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEE, 0xAE, 0xAE, 0xAA, 0xBB, 0xEE, 0xFE}

// TurnOff
{0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xBA, 0xBB, 0xEB, 0xEE, 0xAA, 0xAE, 0xEE, 0xAE, 0xAA, 0xEA, 0xEE, 0xEE, 0xFE}
```

With this knowledge we can connect an own ESP8266 to the XT-pin instead of using the original interface-PCB (see highlighted connector CN501):

![image](https://user-images.githubusercontent.com/9845353/200952754-aaace814-bc4a-4f64-8142-94f9f48cabbc.png)

Electrical and timing-parameters are:
- On connector CN501 (GND, 5V, XT, REC, 5V) the interface-PCB transmit commands on pin XT
- Standby-voltage is +5V and individual bits toggle between +5V and 0V (TTL-level)
- A command is send two times right after each other
- Each bit takes 550µs, so we have a communication with 1800 baud
- Communication starts with 8 bits (4.4ms) at low level (GND), followed by 8 bits (4.4ms) at high-level (5V)
- Communication ends on high-level (5V)

![image](https://user-images.githubusercontent.com/9845353/200953658-b8c471ee-0560-470b-b5ee-80029d5ff555.png)

<a name="outlook"></a>
# Outlook
It would be great to use the available RS485-connection on CON403 using the XYE-pins and the address-encoder to connect multiple device. Anyway, as the RS485 IC is connected to the microcontroller only unidirectional, we do not have any advantages except an easier connection when running long cables. For the moment the found soluation is sufficient for smaller installations with one to four devices within a smaller radius.
