# ChatBoy
Twitch chat client for theButzler's ESP32 GBC Retrofit [see](https://github.com/theBrutzler/ESP32_GBC_RETROFIT).

# Work in progress
Open points:
* faster LCD with double buffering - other library?
* Multithreading / multicore
* why is TTS limited?
* select channel (ChatBoy)
* read cartridge slot
* IR Leds
* Clean up ;)
* transform into a library, add examples
* handle error when SD Card could not be found
* handle PSRAM error

# Used idf-release_v5.3 (Board Manager esp32 by Espressif Systems 3.1.1) and following libraries
* Using library ArduinoIRC at version 0.2.0 in folder: <ARDUINO_HOME>libraries/ArduinoIRC 
* Using library Adafruit ILI9341 at version 1.6.1 in folder: <ARDUINO_HOME>libraries/Adafruit_ILI9341 
* Using library Adafruit GFX Library at version 1.11.11 in folder: <ARDUINO_HOME>libraries/Adafruit_GFX_Library 
* Using library Adafruit BusIO at version 1.17.0 in folder: <ARDUINO_HOME>libraries/Adafruit_BusIO 
* Using library ESP32-audioI2S-master at version 3.0.13 in folder: <ARDUINO_HOME>libraries/ESP32-audioI2S-master 
* Using library JPEGDecoder at version 2.0.0 in folder: <ARDUINO_HOME>libraries/JPEGDecoder 

# Arduino settings
* Board: "ESP32S3 Dev Module"
* Flash size: 8MB
* Partition Scheme: 8M with SPIFFS
* PSRAM: "OPI PSRAM"
