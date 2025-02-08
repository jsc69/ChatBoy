#include "Brutzelboy.h"
#include <Audio.h>
#include <HTTPClient.h>
#include <JPEGDecoder.h>
#include <lcdgfx.h>
#include <lcdgfx_gui.h>

/**
  used library
  - lcdgfx by Alexey Dynda
  - ESP32-audioI2S-master by schreibfaul1
  - JPEGDecoder by Bodmer
*/

#define IMAGE_BUFFER_SIZE 1024
#define SOUND_QUEUE_LENGTH 16

// The parameters are  RST pin, BUS number, CS pin, DC pin, FREQ (0 means default), CLK pin, MOSI pin
DisplayILI9341_240x320x16_SPI tft(RG_GPIO_LCD_RST,{-1, RG_GPIO_LCD_CS, RG_GPIO_LCD_DC, 0, RG_GPIO_LCD_CLK, RG_GPIO_LCD_MOSI});

const String wifiConfig = "/retro-go/config/wifi.json";

Audio audio;
enum SoundType {tts, file, url};
struct Sound {
  SoundType type;
  char source[151];
  char language[2];
};
Sound soundQueue[SOUND_QUEUE_LENGTH];
uint8_t soundPlayed = 255;
uint8_t queuePointer = 255;
bool soundIsPlaying = false;

TaskHandle_t TaskSound;

uint16_t upCount = 0;
uint16_t downCount = 0;
uint16_t leftCount = 0;
uint16_t rightCount = 0;
uint16_t trigger = 300;
uint16_t topBorder = 1600;
uint16_t middleBorder = 1500;
uint16_t bottomBorder = 900;


SPIClass spi = SPIClass();

// Dummy function, if handlers are not used
void doNothing(const uint8_t event, const uint16_t value) {}

void taskPlaySound(void * pvParameters) {
  Brutzelboy *boy = (Brutzelboy *) pvParameters;
  while(true) {
    boy->playQueuedSound();
    delay(100);
  }
}

Brutzelboy::Brutzelboy() {
  keyEventHandler = doNothing;
  soundEventHandler = doNothing;
  hardwareSupport = 255;
}

Brutzelboy::Brutzelboy(uint8_t hardware) {
  keyEventHandler = doNothing;
  soundEventHandler = doNothing;
  hardwareSupport = hardware;
}

void Brutzelboy::begin(const char* ssid, const char* pwd) {
  strcpy(this->ssid, ssid);
  strcpy(this->pwd, pwd);
  begin();
}

void Brutzelboy::begin() {
  if (hardwareSupport & INIT_BUTTONS) {
    pinMode(RG_GPIO_KEY_SELECT, INPUT_PULLUP);
    pinMode(RG_GPIO_KEY_START,  INPUT_PULLUP);
    pinMode(RG_GPIO_KEY_MENU,   INPUT_PULLUP);
    pinMode(RG_GPIO_KEY_OPTION, INPUT_PULLUP);
    pinMode(RG_GPIO_KEY_A,      INPUT_PULLUP);
    pinMode(RG_GPIO_KEY_B,      INPUT_PULLUP);
    pinMode(RG_GPIO_KEY_BOOT,   INPUT_PULLUP);
  }
  pinMode(RG_GPIO_LED,        OUTPUT);
  pinMode(RG_GPIO_LCD_BCKL,   OUTPUT);

  if (hardwareSupport & INIT_LCD) {
    initDisplay();
  } else {
    setLcd(false);
  }
  if (hardwareSupport & INIT_AUDIO) {
    initAudio();
    xTaskCreatePinnedToCore(
                    taskPlaySound,  // Task function
                    "TaskSound",    // name of task
                    10000,          // Stack size of task
                    (void *) this,  // parameter of the task
                    1,              // priority of the task
                    &TaskSound,     // Task handle to keep track of created task
                    0);             // pin task to core 1 
  }
  if (hardwareSupport & INIT_SD_CARD) {
    initSDCard();
    initSPIFFS();
  }
  if (hardwareSupport & INIT_WIFI) {
    initWiFi();
  }
  if (hardwareSupport & INIT_CARTRIDGE) {
    initCartridge();
  }
  if (hardwareSupport & INIT_INFRARED) {
    initInfrared();
  }

  Serial.println("Brutzelboy is ready with support for");
  if (hardwareSupport & INIT_LCD)       Serial.println("\t* Display");
  if (hardwareSupport & INIT_BUTTONS)   Serial.println("\t* Buttons");
  if (hardwareSupport & INIT_SD_CARD)   Serial.println("\t* SD Card");
  if (hardwareSupport & INIT_WIFI)      Serial.println("\t* Wifi");
  if (hardwareSupport & INIT_AUDIO)     Serial.println("\t* Audio");
  if (hardwareSupport & INIT_CARTRIDGE) Serial.println("\t* Cartridge");
  if (hardwareSupport & INIT_INFRARED)  Serial.println("\t* IR Leds");
  Serial.println();
}

void Brutzelboy::initDisplay() {
  tft.begin();
  tft.getInterface().setRotation(1);
  tft.setFixedFont(ssd1306xled_font6x8);
  tft.setColor(RGB_COLOR16(255,255,255));
  tft.fill(RGB_COLOR16(0,0,0));
  setLcd(true);
  hardwareSupport |= INIT_LCD;
}

void Brutzelboy::initWiFi() {
  if (strlen(ssid) == 0 || strlen(pwd) == 0) {
    readWifiConfig();
  }
  if (strlen(ssid) == 0 || strlen(pwd) == 0) {
    Serial.println("ERROR: WIFI could not established. No credentials found!");
    Serial.println("Check that file \"/retro-go/config/wifi.json\" exists on the SDCard and \"ssid0\" and \"password0\" are defined in this file!");
    delay(2000);
    hardwareSupport &= ~INIT_WIFI;
    return;
  }
  Serial.print("Verbinde mit WiFi");
  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Verbunden!");
  hardwareSupport |= INIT_WIFI;
}

void Brutzelboy::initSPIFFS() {
  if (!(hardwareSupport & INIT_SD_CARD)) {
    return;
  }
  while(!SPIFFS.begin(false)){
    Serial.println("ERROR: error has occurred while mounting SPIFFS");
    delay(2000);
  }
}

void Brutzelboy::initAudio() {
  audio.setPinout(RG_GPIO_SND_I2S_BCK, RG_GPIO_SND_I2S_WS, RG_GPIO_SND_I2S_DATA);
  audio.setVolume(16);
}

void Brutzelboy::initSDCard() {
  spi.begin(RG_GPIO_SDSPI_CLK, RG_GPIO_SDSPI_MISO, RG_GPIO_SDSPI_MOSI, RG_GPIO_SDSPI_CS);
  while(!SD.begin(RG_GPIO_SDSPI_CS, spi, 80000000)) {
    Serial.println("ERROR: SD CARD FAILED, OR NOT PRESENT!");
    hardwareSupport &= ~INIT_SD_CARD;
    delay(2000);
  }
}

void Brutzelboy::initCartridge() {
  // TO BE DONE!
    Serial.println("Cartridge is not supported by now");
    hardwareSupport &= ~INIT_CARTRIDGE;
}

void Brutzelboy::initInfrared() {
  // TO BE DONE!
    Serial.println("Infrared leds are not supported by now");
    hardwareSupport &= ~INIT_INFRARED;
}

/************************************************************************
 * SOUND FUNCTION
 ************************************************************************/
void Brutzelboy::playTts(const char* text, const char* language) {
  if (!(hardwareSupport & INIT_AUDIO) || !(hardwareSupport & INIT_WIFI)) {
    Serial.println("ERROR: Hardware does not support function \"playTTS\". Please check initialization of Brutzelboy.");
    return;
  }
  if (!soundIsPlaying) {
    soundIsPlaying = true;
    char buf[150];
    strncpy(buf, text, 150);
    Serial.printf("talk(%d:%d): '%s'\n", soundPlayed % SOUND_QUEUE_LENGTH, queuePointer % SOUND_QUEUE_LENGTH, buf);
    audio.connecttospeech(buf, language);
  }
}

void Brutzelboy::playUrl(const char* url) {
  if (!(hardwareSupport & INIT_AUDIO) || !(hardwareSupport & INIT_WIFI)) {
    Serial.println("ERROR: Hardware does not support function \"playURL\". Please check initialization of Brutzelboy.");
    return;
  }
  if (!soundIsPlaying) {
    soundIsPlaying = true;
    audio.connecttohost(url);
  }
}

void Brutzelboy::playFile(const char* path) {
  if (!(hardwareSupport & INIT_AUDIO) || !(hardwareSupport & INIT_SD_CARD)) {
    Serial.println("ERROR: Hardware does not support function \"playFile\". Please check initialization of Brutzelboy.");
    return;
  }
  if (!soundIsPlaying) {
    soundIsPlaying = true;
    audio.connecttoFS(SD, path);
  }
}

void Brutzelboy::addTtsSoundToQueue(const char* source, const char* language) {
  if (!(hardwareSupport & INIT_AUDIO) || !(hardwareSupport & INIT_WIFI)) {
   Serial.println("ERROR: Hardware does not support function \"addTTSSoundToQueue\". Please check initialization of Brutzelboy.");
   return;
  }
  queuePointer++;
  if (soundPlayed % SOUND_QUEUE_LENGTH == queuePointer % SOUND_QUEUE_LENGTH) {
    queuePointer--;
    Serial.printf("Skip(%d:%d): '%s'\n", soundPlayed % SOUND_QUEUE_LENGTH, queuePointer % SOUND_QUEUE_LENGTH, source);
    return;
  }
  Sound current = soundQueue[queuePointer % SOUND_QUEUE_LENGTH];
  current.type = tts;
  strncpy(current.source, source, 150);
  strncpy(current.language, language, 2);
  soundQueue[queuePointer % SOUND_QUEUE_LENGTH] = current;
}

void Brutzelboy::addFileSoundToQueue(const char* source) {
  if (!(hardwareSupport & INIT_AUDIO) || !(hardwareSupport & INIT_SD_CARD)) {
    Serial.println("ERROR: Hardware does not support function \"addFileSoundToQueue\". Please check initialization of Brutzelboy.");
    return;
  }
  queuePointer++;
  if (soundPlayed % SOUND_QUEUE_LENGTH == queuePointer % SOUND_QUEUE_LENGTH) {
    queuePointer--;
    Serial.printf("Skip(%d:%d): '%s'\n", soundPlayed % SOUND_QUEUE_LENGTH, queuePointer % SOUND_QUEUE_LENGTH, source);
    return;
  }

  Sound current = soundQueue[queuePointer % SOUND_QUEUE_LENGTH];
  current.type = file;
  strcpy(current.source, source);
  soundQueue[queuePointer % SOUND_QUEUE_LENGTH] = current;
}

void Brutzelboy::addUrlSoundToQueue(const char* source) {
  if (!(hardwareSupport & INIT_AUDIO) || !(hardwareSupport & INIT_WIFI)) {
    Serial.println("ERROR: Hardware does not support function \"addUrlSoundToQueue\". Please check initialization of Brutzelboy.");
    return;
  }
  queuePointer++;
  if (soundPlayed % SOUND_QUEUE_LENGTH == queuePointer % SOUND_QUEUE_LENGTH) {
    queuePointer--;
    Serial.printf("Skip(%d:%d): '%s'\n", soundPlayed % SOUND_QUEUE_LENGTH, queuePointer % SOUND_QUEUE_LENGTH, source);
    return;
  }

  Sound current = soundQueue[queuePointer % SOUND_QUEUE_LENGTH];
  current.type = url;
  strcpy(current.source, source);
  soundQueue[queuePointer % SOUND_QUEUE_LENGTH] = current;
}

void Brutzelboy::playQueuedSound() {
  if (!(hardwareSupport & INIT_AUDIO)) {
    Serial.println("ERROR: Hardware does not support function \"playQueuedSound\". Please check initialization of Brutzelboy.");
    return;
  }
  if (soundIsPlaying || soundPlayed % SOUND_QUEUE_LENGTH == queuePointer % SOUND_QUEUE_LENGTH) {
    return;
  }
  soundPlayed++;
  Sound current = soundQueue[soundPlayed % SOUND_QUEUE_LENGTH];

  switch(current.type){
    case tts:
      playTts(current.source, current.language);
      break;
    case file:
      playFile(current.source);
      break;
    case url:
      playUrl(current.source);
      break;
  }
}

void Brutzelboy::setVolume(uint8_t volume) {
  if (!(hardwareSupport & INIT_AUDIO)) {
    Serial.println("ERROR: Hardware does not support function \"setVolume\". Please check initialization of Brutzelboy.");
    return;
  }
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


/************************************************************************
 * DISPLAY FUNCTION
 ************************************************************************/
void Brutzelboy::setColor(const uint8_t r, const uint8_t g, const uint8_t b) {
  if (!(hardwareSupport & INIT_LCD)) {
    Serial.println("ERROR: Hardware does not support function \"setColor\". Please check initialization of Brutzelboy.");
    return;
  }
  tft.setColor(RGB_COLOR16(r, g, b));
}

void Brutzelboy::setBackgroundColor(const uint8_t r, const uint8_t g, const uint8_t b) {
  if (!(hardwareSupport & INIT_LCD)) {
    Serial.println("ERROR: Hardware does not support function \"setBackgroundColor\". Please check initialization of Brutzelboy.");
    return;
  }
  backgroundColor = RGB_COLOR16(r, g, b);
}

void Brutzelboy::printAt(const uint16_t x, const uint16_t y, const char* charStr) {
  if (!(hardwareSupport & INIT_LCD)) {
    Serial.println("ERROR: Hardware does not support function \"printAt\". Please check initialization of Brutzelboy.");
    return;
  }
  tft.printFixed(x, y, charStr, STYLE_NORMAL);
}

void Brutzelboy::clearDisplay() {
  if (!(hardwareSupport & INIT_LCD)) {
    Serial.println("ERROR: Hardware does not support function \"clearLCD\". Please check initialization of Brutzelboy.");
    return;
  }
  tft.fill(backgroundColor);
}

bool Brutzelboy::displayImageFromUrl(const uint16_t x, const uint16_t y, const char* url) {
  if (!(hardwareSupport & INIT_LCD) || !(hardwareSupport & INIT_WIFI)) {
    Serial.println("ERROR: Hardware does not support function \"displayImageFromURL\". Please check initialization of Brutzelboy.");
    return false;
  }
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
        // Speicher erweitern und Daten anh�ngen
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
      renderJpeg(x, y);
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
void Brutzelboy::renderJpeg(const uint16_t offsetX, const uint16_t offsetY) {
  if (!(hardwareSupport & INIT_LCD)) {
    Serial.println("ERROR: Hardware does not support function \"renderJPEG\". Please check initialization of Brutzelboy.");
    return;
  }
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
    uint8_t buffer[2*mcuWidth * mcuHeight];
    for(uint32_t i=0; i<mcuWidth*mcuHeight; i++) {
      uint16_t pixel = JpegDec.pImage[i];
      buffer[2*i]=pixel>>8;
      buffer[2*i+1]=pixel&255;
    }

    tft.drawBitmap16(offsetX + x, offsetY + y, mcuWidth, mcuHeight, buffer);
  }
}


/************************************************************************
 * KEY FUNCTIONS
 ************************************************************************/
void Brutzelboy::checkKeys() {
  if (!(hardwareSupport & INIT_BUTTONS)) {
    Serial.println("ERROR: Hardware does not support function \"checkKeys\". Please check initialization of Brutzelboy.");
    return;
  }
  // Analog Keys
  uint16_t updown = analogRead(RG_ADC_UP_DOWN);
  uint16_t leftright = analogRead(RG_ADC_LEFT_RIGHT);

  if (updown > topBorder) {
    if (upCount <= trigger) upCount++;
    downCount = 0;
  } else if (updown < middleBorder && updown > bottomBorder) {
    if (upCount <= trigger) downCount++;
    upCount = 0;
  } else {
    if (upCount <= trigger) upCount = 0;
    downCount = 0;
  }
  if (leftright > topBorder) {
    if (upCount <= trigger) leftCount++;
    rightCount = 0;
  } else if (leftright < middleBorder && leftright > bottomBorder) {
    if (upCount <= trigger) rightCount++;
    leftCount = 0;
  } else {
    leftCount = 0;
    rightCount = 0;
  }

  processKey(KEY_UP, upCount >= trigger);
  processKey(KEY_DOWN, downCount >= trigger);
  processKey(KEY_LEFT, leftCount >= trigger);
  processKey(KEY_RIGHT, rightCount >= trigger);

  // Digital Keys
  uint8_t gpio=0;
  for (uint16_t i = 16; i<=1024; i=i<<1) {
    switch (i) {
    case KEY_SELECT:
      gpio = RG_GPIO_KEY_SELECT;
      break;
    case KEY_START:
      gpio = RG_GPIO_KEY_START;
      break;
    case KEY_MENU:
      gpio = RG_GPIO_KEY_MENU;
      break;
    case KEY_OPTION:
      gpio = RG_GPIO_KEY_OPTION;
      break;
    case KEY_A:
      gpio = RG_GPIO_KEY_A;
      break;
    case KEY_B:
      gpio = RG_GPIO_KEY_B;
      break;
    case KEY_BOOT:
      gpio = RG_GPIO_KEY_BOOT;
      break;
    }
    processKey(i, !digitalRead(gpio));
  }
}

void Brutzelboy::processKey(uint16_t key, bool pressed) {
  if (pressed) {
    if (!(keys & key)) {
      keyEventHandler(EVENT_KEY_DOWN, key);
    }
    keys |= key;
  } else {
    if (keys & key) {
      keyEventHandler(EVENT_KEY_UP, key);
    }
    keys &= ~key; 
  }
}

bool Brutzelboy::isButtonPressed(const uint16_t key) {
  return keys & key;
}


/************************************************************************
 * LED
 ************************************************************************/
void Brutzelboy::setLed(const bool on) {
  if (on) {
    digitalWrite(RG_GPIO_LED, HIGH);
  } else {
    digitalWrite(RG_GPIO_LED, LOW);
  }
}


/************************************************************************
 * LCD
 ************************************************************************/
void Brutzelboy::setLcd(const bool on) {
  if (!(hardwareSupport & INIT_LCD)) {
    Serial.println("ERROR: Hardware does not support function \"setLcd\". Please check initialization of Brutzelboy.");
    return;
  }
  if (on) {
    digitalWrite(RG_GPIO_LCD_BCKL, HIGH);
  } else {
    digitalWrite(RG_GPIO_LCD_BCKL, LOW);
  }
}


/************************************************************************
 * LOOP
 ************************************************************************/
void Brutzelboy::loop() {
  if (hardwareSupport & INIT_AUDIO) {
    audio.loop();
  }
  if (hardwareSupport & INIT_BUTTONS) {
    checkKeys();
  }
}


/************************************************************************
 * READ CONFIG
 ************************************************************************/
void Brutzelboy::parseCredentialValues(const char* s) {
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
    strcpy(this->ssid, value);
  }
  if (strcmp(key, "password0") == 0) {
    strcpy(this->pwd, value);
  }
}

void Brutzelboy::readWifiConfig() {
  strcpy(ssid, "");
  strcpy(pwd, "");
  if (!(hardwareSupport & INIT_SD_CARD)) {
    return;
  }

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
        parseCredentialValues(buf);
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
