#include "Brutzelboy.h"
#include <lcdgfx.h>
#include <WiFi.h>

// The parameters are  RST pin, {BUS number, CS pin, DC pin, FREQ (0 means default), CLK pin, MOSI pin
DisplayILI9341_240x320x16_SPI tft(RG_GPIO_LCD_RST,{0, RG_GPIO_LCD_CS, RG_GPIO_LCD_DC, 0, RG_GPIO_LCD_CLK, RG_GPIO_LCD_MOSI});

Brutzelboy::Brutzelboy() {
}

void Brutzelboy::begin() {
    initDisplay();
}

void Brutzelboy::initDisplay() {
  tft.begin();
  tft.setFixedFont(ssd1306xled_font8x16);
  tft.fill( 0x0000 );
  tft.getInterface().setRotation(1 & 0x03);
}

void Brutzelboy::initWiFi(String ssid, String password) {
  Serial.print("Verbinde mit WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Verbunden!");
}

DisplayILI9341_240x320x16_SPI Brutzelboy::getLCD() {
  return tft;
}

void Brutzelboy::printAt(uint8_t x, uint8_t y, String str) {
  tft.printFixed(x, y, str.c_str());
}
  
void Brutzelboy::printAt(uint8_t x, uint8_t y, char* charStr) {
  tft.printFixed(x, y, charStr);
}

void Brutzelboy::setFont(const uint8_t* font) {
  tft.setFixedFont(font);
}

void Brutzelboy::setTextColor(uint8_t r, uint8_t g, uint8_t b) {
  tft.setColor(RGB_COLOR16(r, g, b));
}

void Brutzelboy::clearLCD() {
  tft.clear();
}
