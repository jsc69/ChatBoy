#include <IRCClient.h>

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
const String twitchChannelName = "#kekluck";

WiFiClient wifiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wifiClient);

Brutzelboy boy;

char textBuffer[MAX_ROWS][MAX_COLS+1];
uint8_t currentRow = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ChatBoy");

  initTextBuffer();

  boy.begin();
  boy.initWiFi(ssid, password);

  client.setCallback(callback);
  boy.printAt(5, 10, twitchChannelName.c_str());
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
  

  boy.setFont(ssd1306xled_font8x16);
  boy.setTextColor(255, 255, 255);
  boy.clearLCD();
  for (int i=0; i<MAX_ROWS; i++) {
    boy.getLCD().printFixed(5, i*FONT_HEIGHT, textBuffer[i]);
  }
}

void scrollText() {
  for (int i=0; i<MAX_ROWS-1; i++) {
    strcpy(textBuffer[i], textBuffer[i+1]);
  }
  memset(textBuffer[MAX_ROWS-1], ' ', MAX_COLS);
  textBuffer[MAX_ROWS-1][MAX_COLS] = '\0';
}
