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
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson.git)
* [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP.git)
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer.git)

## Setup
* Create a file named `config.h` in the `/src` directory to contain your WIFI's SSID and Password:
``` c++
#define STASSID "ssid"
#define STAPSK "password"
```
* Copy over SSL cert/key (platformio will grab any files in the `/data` folder) to the ESP's filesystem after [creating self signed certificates](https://github.com/me-no-dev/ESPAsyncTCP/blob/master/ssl/gen_server_cert.sh)
```
platformio.exe run --target uploadfs
```
* Apply [fix](https://github.com/me-no-dev/ESPAsyncWebServer/issues/753#issuecomment-616232910) for `server.beginSecure()` to work.


## Wiring
| **Servo**        |   **Wemos D1 Mini**| 
| :-------------: |:-------------: |
| Digital In / Control | D7
| Ground      | Ground       |
| 5v / Vcc | 5v       |

## Pinout

![Wemos D1 Mini Pinout](https://www.projetsdiy.fr/wp-content/uploads/2016/05/esp8266-wemos-d1-mini-gpio-pins.jpg)


## Extra
* [Web Server SSL vs no SSL Response Times Spreadsheet](https://docs.google.com/spreadsheets/d/1TEmO_52ojlhTbK6Bpy6znVFXN7jVqgR1DMHDo9EdO_g/edit?usp=sharing)