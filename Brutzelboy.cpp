#include <cstring>
#include <stdint.h>
#include "HardwareSerial.h"
#include "Brutzelboy.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <Audio.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <JPEGDecoder.h>

/**
  used library
  - Adafruit ILI9341 by Adafruit
  - Adafruit GFX Library by Adafruit
  - ESP32-audioI2S-master by schreibfaul1
  - JPEGDecoder by Bodmer
*/

#define IMAGE_BUFFER_SIZE 1024

Adafruit_ILI9341 tft = Adafruit_ILI9341(RG_GPIO_LCD_CS, RG_GPIO_LCD_DC, RG_GPIO_LCD_MOSI, RG_GPIO_LCD_CLK, RG_GPIO_LCD_RST, RG_GPIO_LCD_MISO);

const String wifiConfig = "/retro-go/config/wifi.json";
char ssid[200];
char pwd[200];

Audio audio;
bool soundIsPlaying = false;

Brutzelboy::Brutzelboy() {
}

void Brutzelboy::begin() {
    pinMode(RG_GPIO_LED, OUTPUT);
    initDisplay();
    initSPIFFS();
    initAudio();
    initSDCard();
    initWiFi();
}

void Brutzelboy::initDisplay() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
}

void Brutzelboy::initWiFi() {
  readWifiConfig();
  if (strlen(ssid) == 0 || strlen(pwd) == 0) {
    Serial.println("Error: WIFI could not established. No credentials found!");
    return;
  }
  Serial.print("Verbinde mit WiFi");
  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Verbunden!");
}

void Brutzelboy::initSPIFFS() {
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void Brutzelboy::initAudio() {
  audio.setPinout(RG_GPIO_SND_I2S_BCK, RG_GPIO_SND_I2S_WS, RG_GPIO_SND_I2S_DATA);
  audio.setVolume(16); // 0...21
}

void Brutzelboy::initSDCard() {
  SPI.begin(RG_GPIO_SDSPI_CLK, RG_GPIO_SDSPI_MISO, RG_GPIO_SDSPI_MOSI, RG_GPIO_SDSPI_CS);
  while (!SD.begin(RG_GPIO_SDSPI_CS)) {
    Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
    delay(1000);
  }
}

uint8_t maxLength = 200; // Google TTS seems to be limited (by time?). So 200 char will be enough.
void Brutzelboy::playTTS(const char* text, const char* language) {
  if (!soundIsPlaying) {
    soundIsPlaying = true;
    char buf[maxLength];
    strlcpy(buf, text, maxLength);
    Serial.printf("talk: '%s'\n", buf);
    audio.connecttospeech(buf, language);
  } else {
    Serial.printf("Skip '%s'\n", text);
  }
}

void Brutzelboy::playURL(const char* url) {
  if (!soundIsPlaying) {
    soundIsPlaying = true;
    audio.connecttohost(url);
  }
}

void Brutzelboy::playFile(const char* path) {
  if (!soundIsPlaying) {
    soundIsPlaying = true;
    audio.connecttoFS(SD, path);
  }
}

void Brutzelboy::setVolume(int volume) {
  if (volume > 21) {
    volume = 21;
  }
  if (volume < 0) {
    volume = 0;
  }
  audio.setVolume(volume);
}

void audio_info(const char *info){
    if (String(info).startsWith("End of")) {
      soundIsPlaying = false;
      Serial.println(info);
    }
}


void Brutzelboy::printDirectAt(uint16_t x, uint16_t y, char* charStr) {
  tft.setCursor(x, y);
  tft.println(charStr);
}

void Brutzelboy::setTextColor(uint8_t r, uint8_t g, uint8_t b) {
  tft.setTextColor(ILI9341_WHITE);
}

void Brutzelboy::clearLCD() {
  tft.fillScreen(ILI9341_BLACK);
}

void Brutzelboy::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool filled) {
  if (filled) {
    tft.fillRect(x, y, w, h, ILI9341_BLACK);
  } else {
    tft.drawRect(x, y, w, h, ILI9341_WHITE);
  }
}


void Brutzelboy::loop() {
    audio.loop();
}


void Brutzelboy::setLed(boolean on) {
  if (on)
    digitalWrite(RG_GPIO_LED, HIGH);
  else
    digitalWrite(RG_GPIO_LED, LOW);
}


bool Brutzelboy::displayImageFromURL(const char* url) {
  HTTPClient http;
  Serial.printf("Loading image from %s\n", url);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    // Lade JPEG-Daten in einen Puffer
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[IMAGE_BUFFER_SIZE];
    size_t totalBytes = 0;

    // Lade alle Daten in einen dynamischen Speicherbereich
    uint8_t* jpgData = nullptr; // Dynamischer Puffer
    size_t jpgSize = 0;

    while (stream->connected() || stream->available()) {
      size_t bytesRead = stream->readBytes(buffer, sizeof(buffer));
      if (bytesRead == 0) {
        break;
      }
      if (bytesRead > 0) {
        // Speicher erweitern und Daten anhängen
        uint8_t* newPtr = (uint8_t*)realloc(jpgData, jpgSize + bytesRead);
        if (newPtr) {
          jpgData = newPtr;
          memcpy(jpgData + jpgSize, buffer, bytesRead);
          jpgSize += bytesRead;
        } else {
          Serial.println("Speicherproblem!");
          free(jpgData);
          http.end();
          return false;
        }
      }
    }
    http.end();

    // JPEG-Daten dekodieren und anzeigen
    if (JpegDec.decodeArray(jpgData, jpgSize)) {
      renderJPEG();
      free(jpgData); // Speicher freigeben
      return true;
    } else {
      Serial.println("Fehler: Konnte JPEG nicht dekodieren.");
      free(jpgData); // Speicher freigeben
    }
  } else {
    Serial.printf("HTTP-Fehler: %d\n", httpCode);
  }

  http.end();
  return false;
}

// Funktion zum Rendern des dekodierten JPEG
void Brutzelboy::renderJPEG() {
  int16_t mcuX = 0, mcuY = 0;

  while (JpegDec.read()) {
    int16_t x = JpegDec.MCUx * JpegDec.MCUWidth;
    int16_t y = JpegDec.MCUy * JpegDec.MCUHeight;
    uint16_t mcuWidth = JpegDec.MCUWidth;
    uint16_t mcuHeight = JpegDec.MCUHeight;

    if ((x + mcuWidth) > JpegDec.width) {
      mcuWidth = JpegDec.width - x;
    }
    if ((y + mcuHeight) > JpegDec.height) {
      mcuHeight = JpegDec.height - y;
    }

    tft.drawRGBBitmap(x, y, JpegDec.pImage, mcuWidth, mcuHeight);
  }
}

void parseValue(const char* s) {
  uint8_t pos[4];
  uint8_t index = 0;

  for(uint8_t i = 0; s[i]!='\0'; i++) {
    if (s[i] == '\"') {
      pos[index++] = i;
    }
  }
  if (index != 4) {
    Serial.printf("Error parsing wifi.json line %s\n", s);
    return;
  }
  char key[1000];
  uint8_t i = 0;
  pos[0]++;
  pos[2]++;
  for (i = 0; i < pos[1]-pos[0] && s[pos[0] + i] != '\0'; i++) {
    key[i] = s[pos[0] + i];
  }
  key[i] = '\0';

  char value[1000];
  for (i = 0; i < pos[3]-pos[2] && s[pos[2] + i] != '\0'; i++) {
    value[i] = s[pos[2] + i];
  }
  value[i] = '\0';

  if (strcmp(key, "ssid0") == 0) {
    strcpy(ssid, value);
  }
  if (strcmp(key, "password0") == 0) {
    strcpy(pwd, value);
  }
}

void Brutzelboy::readWifiConfig() {
  strcpy(ssid, "");
  strcpy(pwd, "");
  File file = SD.open(wifiConfig);
  if(!file){
    Serial.println("wifi.json does not exists");
    return;
  }
  char line[1000];
  uint16_t pos=0;
  while(file.available()){
    int c = file.read();
    if (c == '\n') {
      line[pos] = '\0';
      pos = 0;
      char buf[1000];
      strcpy(buf, line);
      if (strstr(buf, "{") == NULL) {
        parseValue(buf);
      }
    } else {
      line[pos++] = (char)c;
    }
    if (strlen(ssid) > 0 && strlen(pwd) > 0) {
      break;
    }
  }
  file.close();
}


