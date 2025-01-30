#include <IRCClient.h>
#include "Brutzelboy.h"

// Twitch Daten - Bot muss justinfan<magic number> heißen
#define IRC_SERVER          "irc.chat.twitch.tv"
#define IRC_PORT            6667
#define BOT_NAME_PREFIX     "justinfan" // Prefix für annonymen Zugriff
#define TWITCH_OAUTH_TOKEN  ""          // kann leer bleiben bei annonymen Zugriff mit jutinfan.....

#define MAX_ROWS 8
#define MAX_COLS 47
#define FONT_HEIGHT 10

// Name des Channels in Kleinbuchstaben - z.B. "thebrutzler"
const String twitchChannelName = "thebrutzler";
const String urlThumbnail  = "https://static-cdn.jtvnw.net/previews-ttv/live_user_" + twitchChannelName + "-288x162.jpg";

WiFiClient wifiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wifiClient);

Brutzelboy boy;

char textBuffer[MAX_ROWS][MAX_COLS+1];
uint8_t currentRow = 0;

// Timer für das LAden des Thumbnail
const uint32_t interval = 60000;     // 60 Sekunden in Millisekunden
uint32_t previousMillis = -interval; // Zeitstempel der letzten Aktion


SET_LOOP_TASK_STACK_SIZE(8192);

uint16_t flash = 0;
int8_t volume = 16;

void onKeyEvent(const uint8_t event, const uint16_t value) {
  if (event == EVENT_KEY_UP) {
    if (value == KEY_OPTION) {
      if (volume < 21) {
        boy.setVolume(++volume);
      }
      Serial.printf("Volume set to %d\n", volume);
    } else if (value == KEY_MENU) {
      if (volume > 0) {
        boy.setVolume(--volume);
      }
      Serial.printf("Volume set to %d\n", volume);
    }

//    if (value == KEY_UP) Serial.println("UP");
//    if (value == KEY_DOWN) Serial.println("DOWN");
//    if (value == KEY_LEFT) Serial.println("LEFT");
//    if (value == KEY_RIGHT) Serial.println("RIGHT");
    if (value == KEY_A) boy.setLed(true);
    if (value == KEY_B) boy.setLed(false);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ChatBoy");

  initTextBuffer();

  boy.begin();

  client.setCallback(callback);
  boy.printDirectAt(5, 10, "Waiting for thumbnail");

  boy.setKeyEventHandler(onKeyEvent);
}

void loop() {
  client.loop();
  boy.loop();

  if (!client.connected()) {
    connectToTwitch();
  }
  
  if (flash > 0) {
    if (flash % 2000 == 0) {
      boy.setLed(false);
    } else if (flash % 1000 == 0) {
      Serial.println("LED on");
    }
    flash--;
  } else {
    boy.setLed(false);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    boy.displayImageFromURL(urlThumbnail.c_str());
    previousMillis = currentMillis;
  }

  //uint16_t updown = analogRead(7);
  //uint16_t leftright = analogRead(6);
  //Serial.printf("%d - %d\n", updown, leftright);
}

void connectToTwitch() {
  Serial.println("Attempting to connect to " + twitchChannelName );
  char botname[50];
  sprintf(botname, "%s%d", BOT_NAME_PREFIX, random(90000)+10000);
  if (client.connect(botname, "", TWITCH_OAUTH_TOKEN)) {
    Serial.printf("sending JOIN as %s...\n", botname);
    char message[100];
    sprintf(message, "Listening to %s as %s", twitchChannelName, botname);
    printText(message);
//    client.sendRaw("CAP REQ :twitch.tv/tags");
    client.sendRaw("JOIN #" + twitchChannelName);
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
    String talk(ircMessage.nick + " schreibt " + ircMessage.text);
    if (talk.indexOf("!tts") >= 0) {
      talk = String(ircMessage.nick + " sagt " + ircMessage.text);
      talk.replace("!tts", "");
    }
    talk.replace("_", " ");
    boy.playTTS(talk.c_str(), "de");
    
    if (talk.equals("!ttscn ^") || talk.indexOf("jensefu") >= 0) {
      flash=10000;
    }
    if (talk.equals("^blink")) {
      flash=30000;
    }
    
    ircMessage.nick.toUpperCase();
    String message("<" + ircMessage.nick + "> " + ircMessage.text);
    //Serial.println(message);
    printText(message.c_str());
  } else {
    //Serial.println("-->" + ircMessage.command);
  }
}

void initTextBuffer() {
  for (int i=0; i<MAX_ROWS; i++) {
    memset(textBuffer[i], ' ', MAX_COLS);
    textBuffer[i][MAX_COLS] = '\0';
  }
}

void printText(const char* text) {
  uint8_t x=0;
  for(int i=0; i<strlen(text); i++) {
    if (text[i] == 0) {
      break;
    }
    char c = text[i];
    if (x == MAX_COLS) {
      x = 0;
      currentRow++;
    }
    if (currentRow == MAX_ROWS) {
      scrollText();
      currentRow = MAX_ROWS-1;
    }
    textBuffer[currentRow][x] = c;
    x++;
  }
  currentRow++;
  
  boy.drawRect(0, 162, RG_LCD_WIDTH, RG_LCD_HEIGHT, true);
  for (int i=0; i<MAX_ROWS; i++) {
    boy.printDirectAt(5, i*FONT_HEIGHT + 162, textBuffer[i]);
  }
}

void scrollText() {
  for (int i=0; i<MAX_ROWS-1; i++) {
    strcpy(textBuffer[i], textBuffer[i+1]);
  }
  memset(textBuffer[MAX_ROWS-1], ' ', MAX_COLS);
  textBuffer[MAX_ROWS-1][MAX_COLS] = '\0';
}
