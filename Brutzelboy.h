#pragma once
#include <WiFiClient.h>
#include <Adafruit_GFX.h>
#include <FS.h>

#define RG_LCD_WIDTH 288
#define RG_LCD_HEIGHT 240

#define RG_GAMEPAD_ADC1_MAP {\
    {RG_KEY_UP,    ADC1_CHANNEL_6, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_DOWN,  ADC1_CHANNEL_6, ADC_ATTEN_DB_11, 1024, 3072},\
    {RG_KEY_LEFT,  ADC1_CHANNEL_5, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_RIGHT, ADC1_CHANNEL_5, ADC_ATTEN_DB_11, 1024, 3072},\
}

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// Status LED
#define RG_GPIO_LED                 GPIO_NUM_38

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_12
#define RG_GPIO_LCD_CLK             GPIO_NUM_48
#define RG_GPIO_LCD_CS              GPIO_NUM_NC
#define RG_GPIO_LCD_DC              GPIO_NUM_47
#define RG_GPIO_LCD_BCKL            GPIO_NUM_39
#define RG_GPIO_LCD_RST             GPIO_NUM_3

// SPI
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_9
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_11
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_13
#define RG_GPIO_SDSPI_CS            GPIO_NUM_10

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_41
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_42
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_40

// I2C
#define RG_I2C_SDA                  GPIO_NUM_2
#define RG_I2C_CLK                  GPIO_NUM_1

// Serial IO
#define RG_TXD                      GPIO_NUM_43
#define RG_RXD                      GPIO_NUM_44
#define RG_USB_DP                   GPIO_NUM_20
#define RG_USB_DM                   GPIO_NUM_19

// Keys IO
#define RG_GPIO_KEY_SELECT GPIO_NUM_16
#define RG_GPIO_KEY_START  GPIO_NUM_17
#define RG_GPIO_KEY_MENU   GPIO_NUM_18
#define RG_GPIO_KEY_OPTION GPIO_NUM_8
#define RG_GPIO_KEY_A      GPIO_NUM_15
#define RG_GPIO_KEY_B      GPIO_NUM_5
#define RG_GPIO_KEY_BOOT   GPIO_NUM_0
#define RG_ADC_UP_DOWN     7
#define RG_ADC_LEFT_RIGHT  6

// Keys
#define KEY_UP      1
#define KEY_DOWN    2
#define KEY_LEFT    4
#define KEY_RIGHT   8
#define KEY_SELECT  16
#define KEY_START   32
#define KEY_MENU    64
#define KEY_OPTION  128
#define KEY_A       256
#define KEY_B       512
#define KEY_BOOT    1024

// Key events
#define EVENT_KEY_DOWN    1
#define EVENT_KEY_UP      2

// Sound events
#define EVENT_SOUND_START 1
#define EVENT_SOUND_STOP  2


class Brutzelboy {
private:
  void initDisplay();
  void renderJPEG();
  void initAudio();
  void initSPIFFS();
  void initSDCard();
  void readWifiConfig();

  uint16_t keys;
  void checkKeys();
  void processKey(uint16_t key, bool pressed);

  void (*keyEventHandler)(const uint8_t event, const uint16_t value);
  void (*soundEventHandler)(const uint8_t event, const uint16_t value);

public:
  Brutzelboy();
  void begin();
  void loop();
  void initWiFi();

  void setLed(boolean on);
  void setLcd(boolean on);
  
  GFXcanvas16* createCanvas(uint16_t x, uint16_t y);
  void printAt(GFXcanvas16* canvas, uint16_t x, uint16_t y, char* charStr);
  void printDirectAt(uint16_t x, uint16_t y, char* charStr);
  void setTextColor(uint8_t r, uint8_t g, uint8_t b);
  void clearLCD();
  void clearCanvas();
  void drawRect(GFXcanvas16* canvas, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool filled);
  bool displayImageFromURL(const char* url);
  void refreshDisplay(GFXcanvas16* canvas, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

  void addTTSSoundToQueue(const char* source, const char* language);
  void addFileSoundToQueue(const char* source);
  void addUrlSoundToQueue(const char* source);
  void playTTS(const char* text, const char* language);
  void playURL(const char* url);
  void playFile(const char* path);
  void setVolume(int volume);
  void playQueuedSound();

  bool isKeyPressed(const uint16_t key);

  void setKeyEventHandler(void (*userDefinedEventHandler)(const uint8_t event, const uint16_t value)) {
    keyEventHandler = userDefinedEventHandler; }
  void setSoundEventHandler(void (*userDefinedEventHandler)(const uint8_t event, const uint16_t value)) {
    soundEventHandler = userDefinedEventHandler; }

};
