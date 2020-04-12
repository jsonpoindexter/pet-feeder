# pet-feeder

This was project was created using https://platformio.org/

## Components
* USB to Micro USB cable
* [WEMOS D1 Mini](https://wiki.wemos.cc/products:d1:d1_mini)
* [Male 3 Pin JST Connector](https://www.aliexpress.com/item/Free-Shipping-10pcs-3pin-JST-Connector-Male-Female-plug-and-socket-connecting-Cable-Wire-for-WS2811/32366522079.html)

## Project Dependencies
* [ESP8266 WIFI](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
* [NTPClient](https://github.com/arduino-libraries/NTPClient)
* [Servo](https://github.com/esp8266/Arduino/tree/master/libraries/Servo)

## Setup
Create a file named `config.h` in the `/src` directory to contain your WIFI's SSID and Password:
``` c++
#define STASSID "ssid"
#define STAPSK "password"
```

## Wiring
| **Servo**        |   **Wemos D1 Mini**| 
| :-------------: |:-------------: |
| Digital In / Control | D7
| Ground      | Ground       |
| 5v / Vcc | 5v       |

## Pinout

![Wemos D1 Mini Pinout](https://www.projetsdiy.fr/wp-content/uploads/2016/05/esp8266-wemos-d1-mini-gpio-pins.jpg)

