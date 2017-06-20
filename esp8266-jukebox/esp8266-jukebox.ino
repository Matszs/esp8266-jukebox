// source code display: https://github.com/rene-mt/esp8266-oled-sh1106/
// source code rotary encoder: http://bildr.org/2012/08/rotary-encoder-arduino/

#include <Wire.h>
#include "SSD1306.h"
#include <i2s.h>
#include <SPI.h>
#include "Libraries/ArduinoJson/ArduinoJson.h"
#include <ESP8266WiFi.h>
#include "Libraries/Vector/Vector.h"
#include "SH1106SPi.h"
#include "OLEDDisplayUi.h"


struct Songs {
  char id[5];
  char title[30];
  char url[60];
};

// pins
const int encoderPin1 = D3;
const int encoderPin2 = D4;
const int selectButton = 10;
const int oledCLK = D5;
const int oledMISO = D6; // not connected
const int oledMOSI = D7;
const int oledRST = D1;
const int oledDC = D2;
const int oledCS = D8;

// definitions
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
const char* ssid = "jukebox";
const char* password = "esp8266-jukebox";
WiFiClient client;
const char* host = "jukebox.derfu.nl";
Vector<Songs> songList;
String displayText = "esp8266-Jukebox";
String displayTextSecond = "";
SH1106Spi display(oledRST, oledDC);
OLEDDisplayUi ui(&display);
volatile int selectedSong = 0;
volatile boolean play = true;



void setDisplayText(String txt, String secondTxt) {
  displayText = txt;
  displayTextSecond = secondTxt;
  ui.update();
  delay(100);
}

void setDisplayText(String txt) {
  displayText = txt;
  displayTextSecond = "";
  ui.update();
  delay(100);
}

void pinTrigger() {
  int MSB = digitalRead(encoderPin1); // MSB = most significant bit
  int LSB = digitalRead(encoderPin2); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB; // converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; // adding it to the previous encoded value

  if (sum == 0b1101)
    encoderValue++;
  else if (sum == 0b1110)
    encoderValue--;

  lastEncoded = encoded; //store this value for next time

  //displayText = encoderValue;

  selectedSong = (encoderValue / 2);
  //Serial.println((encoderValue / 2));

  play = false;

  if (selectedSong > (songList.size() - 1) && encoderValue > 0) {
    selectedSong = 0;
    encoderValue = 0;
  } else if (selectedSong < 0) {
    selectedSong = songList.size() - 1;
    encoderValue = (selectedSong * 2);
  }

  Serial.println(selectedSong);
}

void selectButtonTrigger() {
  Serial.println("KNOP");
  play = !play;
}

void drawFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_16);

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  if(displayTextSecond.length() > 0) {
    display->drawString(x, 10 + y, displayText);
    display->drawString(x, 27 + y, displayTextSecond);
  } else {
    display->drawString(x, 22 + y, displayText);
  }
}

FrameCallback frames[] = { drawFrame };
int frameCount = 1;

void setupDisplay() {
  ui.setTargetFPS(30);

  ui.disableAllIndicators();
  ui.disableAutoTransition();
  ui.setFrames(frames, frameCount);
  ui.init();

  display.flipScreenVertically();
}

void fillSongListFromRequest(boolean _print) {
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (line.startsWith("\n[")) {
      line.remove(0, 1);
      DynamicJsonBuffer jsonBuffer(512);
      JsonArray& jsonArray = jsonBuffer.parseArray(line);
      for (int i = 0; i < jsonArray.size(); i++ ) {
        struct Songs song;
        strcpy( song.id, jsonArray[i]["id"] );
        strcpy( song.title, jsonArray[i]["title"] );
        strcpy( song.url, jsonArray[i]["url"] );
        songList.push_back(song);
      }
      if (_print) {
        Serial.print("songList length: "); Serial.println(songList.size());
        for (int i = 0; i < songList.size(); i++ ) {
          Serial.print("id:\t"); Serial.println(songList[i].id);
          Serial.print("title:\t"); Serial.println(songList[i].title);
          Serial.print("url:\t"); Serial.println(songList[i].url);
          Serial.println("");
        }
      }
    }
  }
}

void sendRequest(const char* _host, String _url) {
  Serial.print("host: "); Serial.println(_host);
  Serial.print("url: "); Serial.println(_url);
  client.print(String("GET ") + _url + " HTTP/1.1\r\n" +
               "Host: " + _host + "\r\n" +
               "Connection: keep-alive\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}

void connectToWifi(const char* _ssid, const char* _password) {
  Serial.print("Connecting to ");
  Serial.println(_ssid);

  setDisplayText(String("Connecting Wifi"), _ssid);
  delay(100);

  WiFi.begin(_ssid, _password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  setDisplayText(String("IP ") + WiFi.localIP().toString());
  delay(100);

}

bool ICACHE_FLASH_ATTR i2s_write_lr_nb(int16_t left, int16_t right){
  int sample = right & 0xFFFF;
  sample = sample << 16;
  sample |= left & 0xFFFF;
  return i2s_write_sample(sample);
}


void playSong(int songIndex) {

  struct Songs song = songList[songIndex];

  setDisplayText(String("Fetching ") + song.title);


  String url = song.url;
  url.replace("http://", "");
  url.replace(host, "");

  sendRequest(host, url);

  byte lastDataByte = '\0';
  boolean found = false;
  int songDataIndex = 0;
  
  // I2S init code voor audio genereren.
  i2s_begin();
  //pinMode(15, OUTPUT); // lifehacks
  i2s_set_rate(11025);

  while (client.available()) {
    byte dataByte = client.read();

    if (!found && dataByte == '\r' && lastDataByte == '\n') {
      found = true;
      setDisplayText(String("Playing"), song.title);
    }

    if (found && dataByte != '\n') {
      byte dataByteSecond = client.read();

      int16_t pcm = dataByte << 8 | dataByteSecond;

      boolean write = i2s_write_lr_nb(pcm, pcm);
    }

    lastDataByte = dataByte;
  }

  i2s_end();
  setDisplayText(String("Done playing"));
  Serial.println("done");
}

void setup() {
  // Debugging
  Serial.begin(9600);

  WiFi.persistent(false); // prevent exception when connecting to Wifi network

  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  pinMode(selectButton, INPUT);

  // Turn on pull-up resistors
  digitalWrite(encoderPin1, HIGH);
  digitalWrite(encoderPin2, HIGH);
  digitalWrite(selectButton, HIGH);

  attachInterrupt(digitalPinToInterrupt(encoderPin1), pinTrigger, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), pinTrigger, CHANGE);
  attachInterrupt(digitalPinToInterrupt(selectButton), selectButtonTrigger, RISING);

  setupDisplay();

  setDisplayText("Starting...");

  connectToWifi(ssid, password);
  
  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/list.php?json=1";
  sendRequest(host, url);
  fillSongListFromRequest(true);
  
  delay(1000);
}

void loop() {
  if(play) {
    playSong(selectedSong);
    play = false;
  } else {
    struct Songs song = songList[selectedSong];
    setDisplayText(String("Selected "), song.title);
    delay(300);
  }
}
