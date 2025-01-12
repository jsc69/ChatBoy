#include <WiFi.h>
#include <WiFiClient.h>
#include <IRCClient.h>

#include <lcdgfx.h>

#include "Brutzelboy.h"

// Twitch Daten - Bot muss justinfan<magic number> heißen
#define IRC_SERVER          "irc.chat.twitch.tv"
#define IRC_PORT            6667
#define BOT_NAME_PREFIX     "justinfan" // Prefix für annonymen Zugriff
#define TWITCH_OAUTH_TOKEN  ""          // kann leer bleiben bei annonymen Zugriff mit jutinfan.....

#define MAX_ROWS 13
#define MAX_COLS 35
#define FONT_HEIGHT 18

// Anmeldung für WLan. Wenn Daten nicht von <ArduinoHome>/libraries/credentials/credentials.h kommen, 
// nächste Zeile auskommentieren und ssid + password ändern
#include <credentials.h>
String ssid = SSID;
String password = PASSWORD;

// Name des Channels - muss klein geschrieben sein und mit "#" beginnen z.B. "#thebutzler"
const String twitchChannelName = "#thebutzler";

WiFiClient wifiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wifiClient);

// The parameters are  RST pin, {BUS number, CS pin, DC pin, FREQ (0 means default), CLK pin, MOSI pin
DisplayILI9341_240x320x16_SPI tft(RG_GPIO_LCD_RST,{0, RG_GPIO_LCD_CS, RG_GPIO_LCD_DC, 0, RG_GPIO_LCD_CLK, RG_GPIO_LCD_MOSI});
/*
const int canvasWidth = 320;
const int canvasHeight = 240;
uint8_t canvasData[canvasWidth*canvasHeight*2];
NanoCanvas16 canvas(canvasWidth, canvasHeight, canvasData);
*/

char textBuffer[MAX_ROWS][MAX_COLS+1];
uint8_t currentRow = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ChatBoy");

  initDisplay();
  initTextBuffer();

  initWiFi();
  client.setCallback(callback);
  tft.printFixed(5, 10, twitchChannelName.c_str());
}

void initDisplay() {
  tft.begin();
  tft.setFixedFont(ssd1306xled_font8x16);
  tft.fill( 0x0000 );
  tft.getInterface().setRotation(1 & 0x03);
}

void initWiFi() {
  Serial.print("Verbinde mit WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Verbunden!");
}

void loop() {
  if (!client.connected()) {
    connectToTwitch();
    return;
  }
  client.loop();
}

void connectToTwitch() {
  Serial.println("Attempting to connect to " + twitchChannelName );
  String botname = BOT_NAME_PREFIX + String(random(10000-100000));
  if (client.connect(botname, "", TWITCH_OAUTH_TOKEN)) {
    Serial.println("sending JOIN...");
//    client.sendRaw("CAP REQ :twitch.tv/tags");
    client.sendRaw("JOIN " + twitchChannelName);
  } else {
    Serial.println("failed... try again in 5 seconds");
    delay(5000);
  }
}

// not used
void sendTwitchMessage(String message) {
  client.sendMessage(twitchChannelName, message);
}

void callback(IRCMessage ircMessage) {
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
    ircMessage.nick.toUpperCase();
    String message("<" + ircMessage.nick + "> " + ircMessage.text);
    Serial.println(message);
    printText(message);
  } else {
    Serial.println("-->" + ircMessage.command);
  }
}

void initTextBuffer() {
  for (int i=0; i<MAX_ROWS; i++) {
    memset(textBuffer[i], ' ', MAX_COLS);
    textBuffer[i][MAX_COLS] = '\0';
  }
}

void printText(String text) {
  uint8_t x=0;
  for(int i=0; i<text.length(); i++) {
    if (text.charAt(i) == 0) {
      break;
    }
    if (x == MAX_COLS) {
      x = 0;
      currentRow++;
    }
    if (currentRow == MAX_ROWS) {
      scrollText();
      currentRow = MAX_ROWS-1;
    }
    textBuffer[currentRow][x] = text.charAt(i);
    x++;
  }
  currentRow++;
  

  tft.setFixedFont(ssd1306xled_font8x16);
  tft.setColor(RGB_COLOR16(255,255,255));
  tft.clear();
  for (int i=0; i<MAX_ROWS; i++) {
    tft.printFixed(5, i*FONT_HEIGHT, textBuffer[i]);
  }

/*
  canvas.setFixedFont(ssd1306xled_font8x16);
  //canvas.setColor(RGB_COLOR16(0,255,255));
  canvas.clear();
  for (int i=0; i<MAX_ROWS; i++) {
    canvas.printFixed(5, i*fontHeight, textBuffer[i]);
  }
  tft.clear();
  tft.drawCanvas(0, 0, canvas);
*/
}

void scrollText() {
  for (int i=0; i<MAX_ROWS-1; i++) {
    strcpy(textBuffer[i], textBuffer[i+1]);
  }
  memset(textBuffer[MAX_ROWS-1], ' ', MAX_COLS);
  textBuffer[MAX_ROWS-1][MAX_COLS] = '\0';
}
