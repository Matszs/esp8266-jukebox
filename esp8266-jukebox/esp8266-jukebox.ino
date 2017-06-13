// source code display: https://github.com/rene-mt/esp8266-oled-sh1106/
// source code rotary encoder: http://bildr.org/2012/08/rotary-encoder-arduino/

#include <Wire.h>
#include <SPI.h>
#include "SH1106.h"
#include "SH1106Ui.h"


// pins
const int encoderPin1 = D3;
const int encoderPin2 = D4;
const int selectButton = D0;
const int oledCLK = D5;
const int oledMISO = D6;
const int oledMOSI = D7;
const int oledRST = D1;
const int oledDC = D2;
const int oledCS = D8;

// definitions
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
SH1106 display(true, oledRST, oledDC, oledCS);
SH1106Ui ui(&display);


void pinTrigger() {
  int MSB = digitalRead(encoderPin1); // MSB = most significant bit
  int LSB = digitalRead(encoderPin2); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB; // converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; // adding it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderValue ++;
  else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderValue --;

   lastEncoded = encoded; //store this value for next time
}

void setupDisplay() {
  ui.setTargetFPS(30);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawStringMaxWidth(0, 10, 128, "1. Los del Rio - Macarena");

  ui.init();
}

void setup() {
  // Debugging
  Serial.begin(9600);

  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);

  // Turn on pull-up resistors
  digitalWrite(encoderPin1, HIGH);
  digitalWrite(encoderPin2, HIGH);
  
  attachInterrupt(digitalPinToInterrupt(encoderPin1), pinTrigger, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), pinTrigger, CHANGE);

  setupDisplay();
}

void loop() {
  Serial.print("pressed: ");
  Serial.println((digitalRead(selectButton) == HIGH ? "true" : "false"));
  Serial.println("Encoder value: " + encoderValue);

  delay(100);
}
