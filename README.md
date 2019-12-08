# LoRa testing bench with GPS tracking

Ziel dieses Projektes ist es, eine Testumgebung auf Basis von ESP8266 für LoRa als LWPAN-Langstreckenfunk aufzubauen. Dazu wurden zwei identische Testmodule mit jeweils einem LoRa-Transceiver, einem GPS-Modul sowie einem OLED-Display zur Aktualdatenanzeige ausgestattet. Beide Testmodule wurden zunächst auf Breadboards prototypisch aufgebaut. Die Spannungsversorgung erfolgt über den microUSB-Port des ESP8266 (NodeMCU v2) wahlweise angeschlossen an den USB-Port des Projektierungsrechners (gleichzeitig Debugging über serielle Schnittstelle). Alternativ kann auch jeweils eine LiPo-Powerbank verwendet werden, wodurch das Testmodul für anstehende Reichweitentests mobil einsetzbar ist. Als provisorische und outdoor-taugliche Gehäuse dienen wasserdichte Frischhaltedosen.

## Visual impression

(Image Testmodule follows)

(Image Testmodule mit Gehäuse follows)

## Parts list

Following parts I have used in this project:

- breadboard half+ (400 holes)
- wire jumpers
- LiPo battery pack
- USB cable with mini USB plug
- breadboard power supply (with Mini-USB-**Input**)
- Wemos D1 Mini pro (ESP8266)
- Neopixel RGB LED ring, diameter 50 mm (with 12 seperate controllable RGB LEDs by WS2812B chip)
- DFPlayer mini MP3 player module
- microSD card (16 GB)
- passive speaker (2 W, 8 Ohm)
- PIR module (motion sensor based on HC-SR501)
- resistor 1 kOhm (for protecting RxD input of DFPlayer)
- capacitor 1000 µF (protecting the Neopixel LED module)

## Breadboard layout and schematics

With *Fritzing* (https://fritzing.org) I have created following breadboard layout:

![Breadboard Layout](./fritzing/NodeMcu_LoRa_GPS_OLED_Breadboard.png)

The schematics looks like this:

![Schematics](./fritzing/NodeMcu_LoRa_GPS_OLED_Schematics.png)

## Software libraries and documentation


## Todo and known issues

[issue 2019-12-07] Packet message counters flip over sporadicly.








<!--  -->
