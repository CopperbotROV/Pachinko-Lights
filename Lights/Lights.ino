#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <dmx.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

// copyright (c) 2019  David Simmons
// Free to use/copy/modify 
// Don't blame me for bugs, and give credit due

// Some code taken from:
// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
// -Mark Kriegsman, December 2014

// Other code started from:  Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>
// Refer to the "Wire Master Reader" example for use with this
// This example code is in the public domain


#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_DATA_OUT_PIN    9         // Main set of LED lights
#define NUM_LEDS_A    30        // Individual LEDs

CRGB gLedsA[NUM_LEDS_A + 2];         // Color values that are sent to the LED strip.   Add a little extra to prevent a crash


#define LED_CORNER_SQUARE_1   0
#define LED_CORNER_SQUARE_2   1
#define LED_CORNER_SQUARE_3   2
#define LED_TOP_GREEN_FAN     3
#define LED_RIGHT_FLIPPER_1   4
#define LED_RIGHT_FLIPPER_2   5
#define LED_CENTER_RIGHT      6
#define LED_CENTER_BOTTOM     7
#define LED_CENTER_LEFT       8
#define LED_CENTER_TOP        9
#define LED_CENTER_CENTER    10
#define LED_LEFT_FLIPPER_1   11
#define LED_LEFT_FLIPPER_2   12
#define LED_CENTER_BONUS_1   13
#define LED_CENTER_BONUS_2   14
#define LED_RIGHT_TULIP      15
#define LED_RIGHT_PHOENIX    16
#define LED_BOTTOM_BONUS     17
#define LED_LOST_BALL_1      18
#define LED_LOST_BALL_2      19
#define LED_LEFT_PHOENIX     20
#define LED_LEFT_TULIP       21
#define LED_RECYCLE_1        22
#define LED_RECYCLE_2        23
#define LED_BONUS_BALLS_1    24
#define LED_BONUS_BALLS_2    25
#define LED_WIN_TRAY_1       26
#define LED_WIN_TRAY_2       27
#define LED_ANCHOR_1         28
#define LED_ANCHOR_2         29 


uint8_t gBrightness = 96;
uint8_t gHue = 128;

#define FRAMES_PER_SECOND  100

// Debugging mode, dump info to Serial console
bool gDebug = true;

void setup()
{
  Serial.begin(115200);      // open the serial port at 9600 bps
  Serial.print("Pachinko ready to roll\n");
  
  pinMode(LED_DATA_OUT_PIN, OUTPUT);      // Set up control pins

  int i = 0;
  while (i < NUM_LEDS_A)
  {
    gLedsA[i] = CRGB::White;
    i++;
  }
  
  // tell FastLED about the LED strip configuration 
  FastLED.addLeds<LED_TYPE,LED_DATA_OUT_PIN,COLOR_ORDER>(gLedsA, NUM_LEDS_A).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(gBrightness);
  FastLED.show();  

  aliveMessage();
}


void loop() 
{
  EVERY_N_MILLISECONDS( 1000/FRAMES_PER_SECOND )   { stepMachine();   }    // Do stuff
  EVERY_N_SECONDS( 4 )                             { aliveMessage(); }     // Debugging
}


void aliveMessage()
{
  // set master brightness control
  FastLED.setBrightness(gBrightness);

  if (gDebug)
  {
    //Serial.println(F("Pachinko status"));

    // Show the switch status
    //Serial.print(F("   Switch "));
    //Serial.println( digitalRead(LED_SWITCH_PIN) );
  }
}

#define PS_RAINBOW              0
#define PS_MOVE_DOT             1
#define PS_CONFETTI             2
#define PS_MOVE_DOT_PING_PONG   3
#define PS_SINELON              4
#define PS_BPM                  5
#define PS_JUGGLE               6

#define PS_PATTERN_START 0
#define PS_PATTERN_END   6

#define FRAMES_PER_PATTERN  (8 * FRAMES_PER_SECOND)
int     gFrameCounter = 0;
uint8_t gPatternState = PS_PATTERN_START;

// Called at FRAMES_PER_SECOND rate
void stepMachine()
{
  switch(gPatternState)
  {
    case PS_RAINBOW:
      rainbow(gLedsA, NUM_LEDS_A, gHue);
      break;
      
    case PS_MOVE_DOT:
      moveDot(gLedsA, NUM_LEDS_A, gHue);
      break;

    case PS_CONFETTI:
      confetti(gLedsA, NUM_LEDS_A, gHue);
      break;
    
    case PS_MOVE_DOT_PING_PONG:
      moveDotPingPong(gLedsA, NUM_LEDS_A, gHue);
      break;

    case PS_SINELON:
      sinelon(gLedsA, NUM_LEDS_A, gHue);
      break;

    case PS_BPM:
      bpm(gLedsA, NUM_LEDS_A, gHue);
      break;

    case PS_JUGGLE:
      juggle(gLedsA, NUM_LEDS_A, gHue);
      break;
  }
  gHue += 1;   // Don't care about one byte wrap-around

  // Change to next state?
  gFrameCounter++;
  if (gFrameCounter > FRAMES_PER_PATTERN)
  {
    gFrameCounter = 0;
    
    gPatternState++;
    if (gPatternState > PS_PATTERN_END)
    {
      gPatternState = PS_PATTERN_START;
    }
    Serial.print("Pattern is now ");
    Serial.println((int) gPatternState);
  }
  FastLED.show();
}

void setLEDBrightness(uint8_t value)
{
  gBrightness = value;
  FastLED.setBrightness(gBrightness);
}


// Pattern functions
void rainbow(CRGB * rgb_data, int num_leds, uint8_t hue) 
{
  // FastLED's built-in rainbow generator
  fill_rainbow(rgb_data, num_leds, hue, 7);
}

void confetti(CRGB * rgb_data, int num_leds, uint8_t hue) 
{
  // random colored speckles that blink in and fade smoothly
  CRGB * pSeg = rgb_data;
  int seg_size = num_leds;
  uint8_t fade = 255 / seg_size;
  fade = constrain(fade, 10, 64);
  fadeToBlackBy(pSeg, seg_size, fade);
  int pos = random16(seg_size);
  pSeg[pos] += CHSV( hue + random8(64), 200, 255);
}

void moveDotPingPong(CRGB * rgb_data, int num_leds, uint8_t hue)
{
  // a colored dot sweeping back and forth, with fading trails
  CRGB * pSeg = rgb_data;
  int seg_size = num_leds;
  fadeToBlackBy( pSeg, seg_size, 20);
  int pos = beatsin16(13,0,seg_size);
  pSeg[pos] += CHSV(hue, 255, 192);
}

void moveDot(CRGB * rgb_data, int num_leds, uint8_t hue)
{
  // a colored dot sweeping in one direction
  CRGB * pSeg = rgb_data;
  int seg_size = num_leds;
  fadeToBlackBy( pSeg, seg_size, 20);
  float slide_pos = beat16(40);
  int pos = (int)((seg_size * slide_pos) / 65335);
  pSeg[pos] += CHSV(hue, 255, 192);
}

void sinelon(CRGB * rgb_data, int num_leds, uint8_t hue)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( rgb_data, num_leds, 20);
  int pos = beatsin16(13,0,num_leds);
  rgb_data[pos] += CHSV( hue, 255, 192);
}

void bpm(CRGB * rgb_data, int num_leds, uint8_t hue)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < num_leds; i++) { //9948
    rgb_data[i] = ColorFromPalette(palette, hue + (i * 2), beat - hue + (i*10));
  }
}

void juggle(CRGB * rgb_data, int num_leds, uint8_t hue)
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( rgb_data, num_leds, 20);
  byte dot_hue = 0;
  for( int i = 0; i < 8; i++) {
    rgb_data[beatsin16(i+7,0,num_leds)] |= CHSV(dot_hue, 200, 255);
    dot_hue += 32;
  }
}
