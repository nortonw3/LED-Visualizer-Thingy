/**
 *  Author:             Will Norton
 *  Project:            LED Visualizer / Festival Totem
 *  Date Created:       2-6-19
 *  Date Last Modified: 2-14-19 4:42pm
 * 
 *  Description:        TODO
 *  Hardware:           Teensy 3.2, teensy audio shield, 
 *                      3 strips of WS2812B LEDs (78 leds in each)
 * 
 **/

#include <Arduino.h>
#include <FastLED.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Fast LED constants
#define DATA_PIN_1 2
#define DATA_PIN_2 5
#define DATA_PIN_3 7
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 255
#define NUM_LEDS_PER_STRIP 78
#define NUM_STRIPS 3
#define NUM_LEDS (NUM_LEDS_PER_STRIP * NUM_STRIPS);
CRGB strip1[NUM_LEDS_PER_STRIP];
CRGB strip2[NUM_LEDS_PER_STRIP];
CRGB strip3[NUM_LEDS_PER_STRIP];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// FFT Constants / Global Variables
const int BINS = 6;
float level[BINS];
float currentMax[BINS];
int SUB = 0;
int LOWS = 0;
int LOW_MIDS = 0;
int MIDS = 0;
int HIGH_MIDS = 0;
int HIGHS = 0;

// Audio Input Source
//const int myInput = AUDIO_INPUT_LINEIN;
const int myInput = AUDIO_INPUT_MIC;

// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
AudioInputI2S          audioInput;         // audio shield: mic or line-in
AudioAnalyzeFFT1024    fft;
AudioOutputI2S         audioOutput;        // audio shield: headphones & line-out

// Connect either the live input or synthesized sine wave
AudioConnection patchCord1(audioInput, 0, fft, 0);

AudioControlSGTL5000 audioShield;

// Function Prototypes
void getFFT();
void addGlitter();
void spectrumAnalizer();
void stars();

void setup() {
  // Setup LED strips here
  FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(strip1, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(strip2, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(strip3, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(BRIGHTNESS);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);

  // Configure the window algorithm to use
  fft.windowFunction(AudioWindowHanning1024);
  //myFFT.windowFunction(NULL);

  // for debug, I set each bin to 400 and leave it
  // normally I'd replace with the max per bin seen as the audio streams
   for(int i = 0; i < 16; i++) { currentMax[i] = 400; }
}

void loop() {
  // put your main code here, to run repeatedly:
  getFFT();
  //spectrumAnalizer();
  stars();
  FastLED.show();
  EVERY_N_MILLISECONDS( 40 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

// this function is what does all the FFT reading
void getFFT() {
  if (fft.available()) {
    // Play with Values here to dial in the ranges for each BIN
    level[0] = fft.read(1, 2);
    level[1] = fft.read(3, 5);
    level[2] = fft.read(6, 11);
    level[3] = fft.read(12, 23);
    level[4] = fft.read(39, 89);
    level[5] = fft.read(164, 500);

    // This is where i would average the max readings for scaling the FFT readings on the fly. 
    // it is not complete yet
    // float max[BINS]       = {0, 0, 0, 0, 0, 0};
    // float prevVals[BINS]  = {0, 0, 0, 0, 0, 0};

    // //Set Max Values for each BIN
    // for (int i = 0; i < BINS; i++) {
    //   if (level[i] > max[i] && level[i] > prevVals[i]) {
    //     max[i] = level[i];
    //     EVERY_N_MILLISECONDS(100) {max[i] -= 0.01;}
    //     currentMax[i] = max[i] * 100;
    //   }
    // }

    // Used for debugging, prints BIN values to Serial Monitor
    // for (int i = 0; i < BINS; i++) {
    //   Serial.print(currentMax[i]);
    //   Serial.print(", ");
    // }
    // Serial.println();
  }
}

void addGlitter( fract8 chanceOfGlitter) {             // Let's add some glitter, thanks to Mark
  
  if( random8() < chanceOfGlitter) {
    strip1[random16(NUM_LEDS_PER_STRIP)] += CRGB::White;
    strip2[random16(NUM_LEDS_PER_STRIP)] += CRGB::White;
    strip3[random16(NUM_LEDS_PER_STRIP)] += CRGB::White;
  }

} // addGlitter()

// This function only uses two strips atm bit it shows 6 diffetent frequency ranges
void spectrumAnalizer() {
  SUB = constrain(map(level[0] * 1000, 30, int(currentMax[0]), 0, NUM_LEDS_PER_STRIP/3), 0, NUM_LEDS_PER_STRIP/3);
  LOWS = constrain(map(level[1] * 1000, 30, int(currentMax[1]), 0, NUM_LEDS_PER_STRIP/3), 0, NUM_LEDS_PER_STRIP/3);
  LOW_MIDS = constrain(map(level[2] * 1000, 30, int(currentMax[2]), 0, NUM_LEDS_PER_STRIP/3), 0, NUM_LEDS_PER_STRIP/3);
  MIDS = constrain(map(level[3] * 1000, 30, int(currentMax[3]), 0, NUM_LEDS_PER_STRIP/3), 0, NUM_LEDS_PER_STRIP/3);
  HIGH_MIDS = constrain(map(level[4] * 1000, 30, int(currentMax[4]), 0, NUM_LEDS_PER_STRIP/3), 0, NUM_LEDS_PER_STRIP/3);
  HIGHS = constrain(map(level[5] * 1000, 30, int(currentMax[5]), 0, NUM_LEDS_PER_STRIP/3), 0, NUM_LEDS_PER_STRIP/3);
    
  // color for each frequency range
  fill_solid(strip1, SUB, CRGB(92, 0, 150));
  fill_solid(strip2, LOWS, CRGB(0, 28, 214));
  fill_solid(strip1 + (NUM_LEDS_PER_STRIP/3), LOW_MIDS, CRGB(0, 237, 255));
  fill_solid(strip2 + (NUM_LEDS_PER_STRIP/3), MIDS, CRGB(0, 255, 0));
  fill_solid(strip1 + 2 * (NUM_LEDS_PER_STRIP/3), HIGH_MIDS, CRGB(250, 255, 0));
  fill_solid(strip2 + 2 * (NUM_LEDS_PER_STRIP/3), HIGHS, CRGB(255, 0, 0));

  // Fade Each Pixel out gradually
  fadeToBlackBy(strip1, NUM_LEDS_PER_STRIP, 20);
  fadeToBlackBy(strip2, NUM_LEDS_PER_STRIP, 20);
}

// this function uses all 3 strips and maps 3 separate frequency ranges to pulsing color bars while the high frequencies 
// trigger the addGlitter() function
void stars() {
  SUB = constrain(map(level[1] * 1000, 30, int(currentMax[1]), 0, NUM_LEDS_PER_STRIP), 0, NUM_LEDS_PER_STRIP);
  MIDS = constrain(map(level[2] * 1000, 30, int(currentMax[2]), 0, NUM_LEDS_PER_STRIP), 0, NUM_LEDS_PER_STRIP);
  HIGH_MIDS = constrain(map(level[3] * 1000, 30, int(currentMax[3]), 0, NUM_LEDS_PER_STRIP), 0, NUM_LEDS_PER_STRIP);
  
  HIGHS = constrain(map(level[5] * 1000, 30, int(currentMax[5]), 0, NUM_LEDS_PER_STRIP), 0, NUM_LEDS_PER_STRIP);

  fill_solid(strip1, SUB,  CHSV( gHue, 255, 192));
  fill_solid(strip2, MIDS, CHSV( gHue + 15, 255, 192));
  fill_solid(strip3, HIGH_MIDS, CHSV( gHue + 30, 255, 192));

  fadeToBlackBy(strip1, NUM_LEDS_PER_STRIP, 25);
  fadeToBlackBy(strip2, NUM_LEDS_PER_STRIP, 25);
  fadeToBlackBy(strip3, NUM_LEDS_PER_STRIP, 25);

  addGlitter(HIGHS);
}


