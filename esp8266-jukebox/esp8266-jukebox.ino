// source code display: https://github.com/rene-mt/esp8266-oled-sh1106/
// source code rotary encoder: http://bildr.org/2012/08/rotary-encoder-arduino/


#include <Wire.h>
#include "SSD1306.h"
#include <SPI.h>
#include "SH1106SPi.h"

#include "OLEDDisplayUi.h"


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
    encoderValue --;*/

    if(sum == 0b1101)
      encoderValue++;
    else if(sum == 0b1110)
      encoderValue--;

   lastEncoded = encoded; //store this value for next time

   displayText = encoderValue;
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
}

void loop() {
  //Serial.println((digitalRead(selectButton) == HIGH ? "true" : "false"));
  //Serial.println("Loop");
  ui.update();
}
