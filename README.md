# LoRa testing bench with GPS tracking

<!-- Ziel dieses Projektes ist es, eine Testumgebung auf Basis von ESP8266 für LoRa als LPWAN-Langstreckenfunk aufzubauen. -->
The aim of this project is to build a test environment based on ESP8266 for [LoRa](https://en.wikipedia.org/wiki/LoRa) as [LPWAN](https://en.wikipedia.org/wiki/LPWAN) long-range radio.

<!-- Dazu wurden zwei identische Testmodule mit jeweils einem LoRa-Transceiver, einem GPS-Modul sowie einem OLED-Display zur Aktualdatenanzeige ausgestattet. -->
For this purpose, two identical test modules were each equipped with a LoRa transceiver, a GPS module and an OLED display for displaying the actual data.

<!-- Beide Testmodule wurden zunächst auf Breadboards prototypisch aufgebaut. Die Spannungsversorgung erfolgt über den microUSB-Port des ESP8266 (NodeMCU v2) und wahlweise angeschlossen an den USB-Port des Projektierungsrechners. Damit kann gleichzeitig ein Debugging über die serielle Schnittstelle und den Serial Monitor erfolgen. -->
Both test modules were initially prototypically built on breadboards. Power is supplied via the microUSB port of the ESP8266 (NodeMCU v2) and optionally connected to the USB port of the configuration computer. This allows debugging via the serial interface and the serial monitor at the same time.

<!-- Alternativ kann auch für beide Testmodule eine LiPo-Powerbank verwendet werden, wodurch sie für anstehende Reichweitentests mobil einsetzbar ist. -->
Alternatively, a LiPo power bank can also be used for both test modules, making them mobile for upcoming range tests.

<!-- Als provisorische und outdoor-taugliche Gehäuse dienen im Moment wasserdichte Frischhaltedosen. -->
At the moment, watertight food storage boxes are used as provisional and outdoor-suitable housings.

## Visual impressions

Here you can see one of two test modules build on breadboard:

![Breadboard Layout](./doc/images/Testboard.jpeg)

In this picture you can see the other test module on top of the provisional and outdoor-suitable housing (watertight food storage box):

![Breadboard Layout and case](./doc/images/Testboard_case.jpeg)

## Parts list

Following parts I have used in this project (every component you will need twice of course):

- breadboard full+ (830 holes)
- wire jumpers
- LiPo battery pack
- USB cable with micro USB plug
- NodeMCU v2 (ESP8266)
- OLED I2C display, 128 x 64 Pixel, 1.3 Zoll (SSH1106 chip)
- GPS module u-blox Neo-6M (GY-GPS6MV2 chip)
- LoRa transceiver Adafruit RFM9x LoRa Radio (RFM95W chip, 868 MHz)
- LoRa antenna (868 MHz), 3 dBi omnidirectional, SMA plug

## Breadboard layout and schematics

With *Fritzing* (https://fritzing.org) I have created following breadboard layout:

![Breadboard Layout](./fritzing/NodeMcu_LoRa_GPS_OLED_Breadboard.png)

The schematics looks like this:

![Schematics](./fritzing/NodeMcu_LoRa_GPS_OLED_Schematics.png)

## Software libraries and documentation


## License

This project is licensed under the terms of "GNU General Public License v3.0". For details see [LICENSE](LICENSE).

<!-- ## Todo and known issues

[issue 2019-12-07] Packet message counters flip over sporadically (this may be due to crashes of the mcu). -->








<!--  -->
