// source code display: https://github.com/rene-mt/esp8266-oled-sh1106/
// source code rotary encoder: http://bildr.org/2012/08/rotary-encoder-arduino/


#include <Wire.h>
#include "SSD1306.h"
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
String displayText = "kaas";
SH1106Spi display(oledRST, oledDC);
OLEDDisplayUi ui(&display);

void pinTrigger() {
  int MSB = digitalRead(encoderPin1); // MSB = most significant bit
  int LSB = digitalRead(encoderPin2); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB; // converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; // adding it to the previous encoded value

  //Serial.println(sum, BIN);

  /*if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderValue ++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
<<<<<<< HEAD
    encoderValue --;
    
  lastEncoded = encoded; //store this value for next time
  
  displayText = encoderValue;
=======
    encoderValue --;*/

    if(sum == 0b1101)
      encoderValue++;
    else if(sum == 0b1110)
      encoderValue--;

   lastEncoded = encoded; //store this value for next time

   displayText = encoderValue;
>>>>>>> 10dbc4993d1e184b43d438be030b990eab7dfa8e
}

void selectButtonTrigger() {
  displayText = "KNOP";
  Serial.println("KNOP");
}

void drawFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_16);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(86 + x, 22 + y, displayText);
}

FrameCallback frames[] = { drawFrame };

// how many frames are there?
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
  client.print(String("GET ") + _url + " HTTP/1.1\r\n" +
               "Host: " + _host + "\r\n" +
               "Connection: close\r\n\r\n");
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

  WiFi.begin(_ssid, _password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  // Debugging
  Serial.begin(9600);

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

  connectToWifi(ssid, password);
  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/list.php?json=1";
  sendRequest(host, url);
  fillSongListFromRequest(true);
}

void loop() {
  //Serial.println((digitalRead(selectButton) == HIGH ? "true" : "false"));
  //Serial.println("Loop");
  ui.update();
}
