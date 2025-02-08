# ChatBoy
Twitch chat client for theButzler's ESP32 GBC Retrofit [see](https://github.com/theBrutzler/ESP32_GBC_RETROFIT).

# Work in progress
Open points:
* add support double buffering for lcd
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
* Using library lcdgfx at version 1.1.5 
* Using library ESP32-audioI2S-master at version 3.0.13
* Using library JPEGDecoder at version 2.0.0 in folder

# Arduino settings
* Board: "ESP32S3 Dev Module"
* Flash size: 8MB
* Partition Scheme: 8M with SPIFFS
* PSRAM: "OPI PSRAM"
