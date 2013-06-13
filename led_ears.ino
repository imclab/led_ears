// Fox Ears sketch rewrite v2
#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>
#include "utilities.h"

// default settings for different people:
#define HueDefaultBluebie 85
#define HueDefaultDresona 42
#define HueDefault HueDefaultBluebie
// note that hues range from blue at 0, through to purple, red, orange, green, aqua, blue again at 255

// constants used to identify different groups of pixels
#define EarsInner 1
#define EarsEdge 0

#define PrimaryButton 5
#define SecondaryButton 1

// library used to communcate to these LED pixels in the loop function:
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(6, 0, NEO_RGB + NEO_KHZ400);
// currently selected program:
byte selected_animation_idx = 0;
// list of available programs
PGM_P animations[] PROGMEM = {
  (PGM_P) solid,
  (PGM_P) edge,
  (PGM_P) inner,
  (PGM_P) heartbeat,
  (PGM_P) gradient_edge,
  (PGM_P) opposites,
  (PGM_P) rainbow_opposites,
  (PGM_P) white,
  (PGM_P) strobe,
  (PGM_P) alternating_strobe,
  (PGM_P) bicolor_strobe,
  (PGM_P) forest_walk,
  (PGM_P) random_walk,
  (PGM_P) wave,
  (PGM_P) oceanic,
};

struct pixel_request {
  byte idx;
  byte kind;
  byte height;
};

byte light_heights[] PROGMEM = {20, 0, 10, 10, 0, 20};

// some state stuff
RGBPixel primary_color;
byte primary_color_hue = HueDefault;

void setup() {
  // set our two input buttons high
  digitalWrite(5, HIGH);
  digitalWrite(1, HIGH);
  
  pixels.begin();
}

void loop() {
  // load the current time in milliseconds
  unsigned long time = millis();
  
  primary_color = color_wheel(primary_color_hue);
  
  // detect when button is pressed down
  handle_primary_button(time);
  handle_secondary_button(time);
  
  // load our currently selected function
  //RGBPixel (*program_function)(unsigned long time, byte pixel_idx, byte pixel_kind)
  //  = (RGBPixel(*)(unsigned long, byte, byte)) pgm_read_word(programs + selected_program_idx);
  RGBPixel (*program_function)(struct pixel_request, unsigned long)
    = (RGBPixel(*)(struct pixel_request, unsigned long)) pgm_read_word(animations + selected_animation_idx);
  
  // ask function for colours of all our LEDs
  struct pixel_request pixel; // variable to store our requests
  for (pixel.idx = 0; pixel.idx < pixels.numPixels(); pixel.idx++) {
    pixel.kind = (pixel.idx == 2 || pixel.idx == 3) ? EarsInner : EarsEdge;
    //pixel.height = pixel.idx < 3 ? pixel.idx * 10 : 50 - (pixel.idx * 10);
    pixel.height = pgm_read_byte(&light_heights[pixel.idx]);
    RGBPixel color = program_function(pixel, time);
    if (color != CURRENT_COLOR) pixels.setPixelColor(pixel.idx, color);
  }
  
  // send new colours to LEDs
  pixels.show();
}

void handle_primary_button(byte time) {
  Debounce(time, digitalRead(PrimaryButton));
  static boolean recharged;
  
  if (!digitalRead(PrimaryButton)) {
    if (recharged) {
      selected_animation_idx += 1;
      if (selected_animation_idx >= (sizeof(animations) / sizeof(PGM_P))) selected_animation_idx = 0;
      indicate(color_wheel(HueDefault)); //RGB(255, 255, 255));
      recharged = false;
    }
  } else {
    recharged = true;
  }
}

#define HueSelectDiv 20
void handle_secondary_button(unsigned int time) {
  Debounce(time, digitalRead(PrimaryButton));
  if (!digitalRead(SecondaryButton)) {
    static byte prev_time;
    if (prev_time != (time / HueSelectDiv) % 256) {
      prev_time = (time / HueSelectDiv);
      primary_color_hue += 1;
    }
  }
}

void indicate(RGBPixel color) {
  for (int i = 255; i >= 0; i--) {
    for (byte pixel = 0; pixel < pixels.numPixels(); pixel++) {
      pixels.setPixelColor(pixel, multiply_colors(color, GRAY(i)));
    }
    pixels.show();
  }
}