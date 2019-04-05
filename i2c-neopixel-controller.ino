/*******************************************************************************
 *** i2c-neopixel-controller.ino
 ***
 *** Copyright (c) 2019 Ben Bleything <ben@bleything.net>
 *** Released under the terms of the MIT License; see LICENSE for details
 ******************************************************************************/

#include <Adafruit_NeoPixel.h>
#include <Wire.h>

////////////////////////////////////////////////////////////////////////////////
/// Configuration
////////////////////////////////////////////////////////////////////////////////

// i2c addressing is a free-for-all, so feel free to change this if you have
// another device that conflicts
#define I2C_ADDRESS 0x45

// the pin that's connected to DIN on your NeoPixels
#define NEOPIXEL_PIN 1

// self-explanatory
#define NEOPIXEL_COUNT 9

// the constant from the NeoPixel library that matches your particular
// strip. This can be a million different values; common ones are NEO_GRB (most
// Adafruit NeoPixel products) and NEO_GRBW (if you've got RGBW LEDs). Change
// this if you're experiencing weird results.
#define NEOPIXEL_TYPE NEO_GRB

////////////////////////////////////////////////////////////////////////////////
/// Defaults
////////////////////////////////////////////////////////////////////////////////

// sets the maximum brightness of the pixels; 40 is a reasonable default for
// direct viewing without searing your retinas
byte brightness = 40;

////////////////////////////////////////////////////////////////////////////////
/// You shouldn't need to change anything below here, but feel free!
////////////////////////////////////////////////////////////////////////////////

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEOPIXEL_TYPE + NEO_KHZ800);

// we keep the pixel state in memory so it can be manipulated very quickly in
// the command handlers. The updatePixels() function iterates over this and
// sends commands to the LEDs
byte pixelState[NEOPIXEL_COUNT][3];

void setup() {
  // for debugging!
  Serial.begin(115200);

  Wire.begin(I2C_ADDRESS);

  // registers a callback for when i2c messages are received
  Wire.onReceive(receiveEvent);

  // initialize and blank the strip
  pixels.begin();
  pixels.setBrightness(brightness);
  pixels.show();

  // this just initializes pixelState
  blackout();
}

// all of the work is handled in receiveEvent, so loop doesn't do anything
void loop() {}

////////////////////////////////////////////////////////////////////////////////
/// Helper Methods
////////////////////////////////////////////////////////////////////////////////

// iterates over the state array and sets everything to zero
void blackout() {
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixelState[i][0] = 0;
    pixelState[i][1] = 0;
    pixelState[i][2] = 0;
  }
}

// dumps the i2c buffer as hex bytes
void dumpMessage() {
  Serial.print("[ ");

  while (Wire.available()) {
    char buf[4];
    sprintf(buf, "0x%02X ", Wire.read());
    Serial.print(buf);
  }

  Serial.println("]");
}

// fetches the color of a pixel from the state array and returns it as a
// uint32_t that can be passed directly to pixels.setPixelColor()
uint32_t pixelColor(int pixel) {
  byte r = pixelState[pixel][0];
  byte g = pixelState[pixel][1];
  byte b = pixelState[pixel][2];

  return pixels.Color(r, g, b);
}

// copies the pixelState array to the actual LEDs. Also sets brightness.
void updatePixels() {
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixelColor(i));
  }

  pixels.setBrightness(brightness);
  pixels.show();
}

////////////////////////////////////////////////////////////////////////////////
/// Command Handler
////////////////////////////////////////////////////////////////////////////////

// this callback is fired whenever we receive an i2c message
//
// the command "parser" is deliberately very simple... it works well enough for
// me for now but I'm happy to make it better if it doesn't work for you. File
// an issue or PR!
//
// Commands:
//
// * 0x00 - turn off all the LEDs
// * 0x01 - set the maximum brightness. Takes one arg.
// * 0x02 - set a specific LED to the given color. takes four args: addr, red,
//          green, blue
void receiveEvent(int numBytes) {
  int command = Wire.read();

  switch (command) {
  case 0x00:
    Serial.println("blackout");
    blackout();
    break;

  case 0x01:
    brightness = Wire.read();
    Serial.print("set max brightness to ");
    Serial.println(brightness);
    break;

  case 0x02:
    // do a little bit of sanity checking... if we got a weird number of bytes
    // it's better to just bail than to guess what the user meant
    if (numBytes != 5) {
      Serial.print("received 0x02 (setLED) but the wrong number of args. expected 4; got ");
      Serial.print(numBytes - 1);

      if(numBytes - 1) {
        Serial.print(": ");
        dumpMessage();
      } else {
        // dumpMessage() does a println so we only need it if we didn't end up
        // calling dumpMessage()
        Serial.println();
      }

    } else {
      int led = Wire.read();
      int r   = Wire.read();
      int g   = Wire.read();
      int b   = Wire.read();

      char buffer[30];
      sprintf(buffer, "setLED %i { %i, %i, %i }", led, r, g, b);
      Serial.println(buffer);

      pixelState[led][0] = r;
      pixelState[led][1] = g;
      pixelState[led][2] = b;
    }
    break;

  default:
    char buf[30];
    sprintf(buf, "Received unknown command 0x%02X", command);
    Serial.print(buf);

    if(Wire.available()) {
      Serial.print(" with args ");
      dumpMessage();
    } else {
      // as above... dumpMessage() does a println so we only need it if we
      // didn't end up calling dumpMessage()
      Serial.println();
    }
  }

  updatePixels();

  // dump the rest of the buffer
  Wire.flush();
}
