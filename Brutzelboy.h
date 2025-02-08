#pragma once
#include <cstdint>

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

// Hardware Flags
#define INIT_LCD          1
#define INIT_BUTTONS      2
#define INIT_SD_CARD      4
#define INIT_WIFI         8
#define INIT_AUDIO        16
#define INIT_CARTRIDGE    32
#define INIT_INFRARED     64

// Key events
#define EVENT_KEY_DOWN    1
#define EVENT_KEY_UP      2

// Sound events
#define EVENT_SOUND_START 1
#define EVENT_SOUND_STOP  2


class Brutzelboy {
private:
  void initDisplay();
  void initAudio();
  void initSPIFFS();
  void initSDCard();
  void readWifiConfig();
  void initWiFi();
  void initCartridge();
  void initInfrared();

  uint8_t hardwareSupport = 0;
  bool isHardwareSupported(const uint8_t hardware) {
     return hardwareSupport & hardware;
  }

  uint16_t keys;
  void checkKeys();
  void processKey(uint16_t key, bool pressed);

  void (*keyEventHandler)(const uint8_t event, const uint16_t value);
  void (*soundEventHandler)(const uint8_t event, const uint16_t value);

  uint16_t backgroundColor;
  void renderJpeg(const uint16_t x, uint16_t y);

  void parseCredentialValues(const char* s);
  char ssid[200];
  char pwd[200];

public:
  /**
   * @brief Constuctor for a Brutzelboy instance with chosen hardware support.
   * @param uint8_t Flgs of hardware that should be supported. See Hardware Flags (#define INIT_xxx)
   */
  Brutzelboy(uint8_t hardware);
  /**
   * @brief Constuctor for a Brutzelboy instance with complete hardware supprt.
   */
  Brutzelboy();

  /**
   * @brief Init the Brutzelboy with wifi credentials from SDCard. Should be called in setup()
   */
  void begin();
  /**
   * @brief Init the Brutzelboy with given wifi credentials. Should be called in setup()
   */
  void begin(const char* ssid, const char* pwd);
  /**
   * @brief Loop for the Brutzelboy.Should call frequently to allow the Brutzelboy doing its thing. Call it in loop()
   */
  void loop();

  /**
   * @brief Set the green led on the left side of the display
   * @param bool
   */
  void setLed(const bool on);
  /**
   * @brief Set the background light of the display
   * @param bool
   */
  void setLcd(const bool on);
  
  /**
   * @brief Print text on the given position
   * @param uint16_t x position
   * @param uint16_t y position
   * @param char* text
   */
  void printAt(const uint16_t x, const uint16_t y, const char* charStr);
  /**
   * @brief Set the drawing color in RGB. Default: write
   * @param uint8_t red
   * @param uint8_t green
   * @param uint8_t blue
   */
  void setColor(const uint8_t r, const uint8_t g, const uint8_t b);
  /**
   * @brief Set the background color in RGB. Default: black
   * @param uint8_t red
   * @param uint8_t green
   * @param uint8_t blue
   */
  void setBackgroundColor(const uint8_t r, const uint8_t g, const uint8_t b);
  /**
   * @brief Clear the display by filling it in the background color
   */
  void clearDisplay();
  /**
   * @brief Draw a jpeg image loaded from web at given position
   * @param uint16_t left edge of refresh area (framebuffer and display)
   * @param uint16_t top edge of refresh area (framebuffer and display)
   * @param char* url of jpeg
   * @return bool true, if image could be loaded and displayed successfully
   */
  bool displayImageFromUrl(const uint16_t x, const uint16_t y, const char* url);
  /**
   * @brief Clear the framebuffer by filling it in the background color
   */
  void clearFramebuffer();

  /**
   * @brief Add a text in queue to be play via TTS
   * @param char* text to be play
   * @param char* language to use in TTS
   */
  void addTtsSoundToQueue(const char* source, const char* language);
  /**
   * @brief Add a sound file from SDCard in queue to be played
   * @param char* filename incl. absolute path
   */
  void addFileSoundToQueue(const char* source);
  /**
   * @brief Add a sound file from web in queue to be played
   * @param char* url of sound file
   */
  void addUrlSoundToQueue(const char* source);
  /**
   * @brief Play text directly via TTS
   * @param char* text to play
   * @param char* language to use in TTS
   */
  void playTts(const char* text, const char* language);
  /**
   * @brief Play sound file from web directly
   * @param char* url of sound file
   */
  void playUrl(const char* url);
  /**
   * @brief Play sound file from SDCard directly
   * @param char* filename incl. absolute path
   */
  void playFile(const char* path);
  /**
   * @brief Set the volume (0-21)
   * @param uint8_t new volume
   */
  void setVolume(uint8_t volume);
  /**
   * @brief Start playing next sound from queue if no sound is playing
   */
  void playQueuedSound();

  /**
   * @brief Check if a button is pressed
   * @param uint16_t id of button
   * @return true if pressed
   */
  bool isButtonPressed(const uint16_t key);
  /**
   * @brief Add handler for button event
   * @param void (*userDefinedEventHandler) function to handle button events
   */
  void setKeyEventHandler(void (*userDefinedEventHandler)(const uint8_t event, const uint16_t value)) {
    keyEventHandler = userDefinedEventHandler; }
  /**
   * @brief Add handler for sound event
   * @param void (*userDefinedEventHandler) function to handle sound events
   */
  void setSoundEventHandler(void (*userDefinedEventHandler)(const uint8_t event, const uint16_t value)) {
    soundEventHandler = userDefinedEventHandler; }

};
