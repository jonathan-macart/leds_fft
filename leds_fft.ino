/* LED Output: FFT of Audio Signal
               (Spectrum Analyzer)
         
   Author: J.F. MacArt
   Date:   10 Dec 2016
*/

#include <EEPROM.h>
#include<fix_fft.h>
#include<FastLED.h>

#define MAX_BRIGHT 255

#define NUM_LEVELS 6
#define num_hist 2
#define pin_adc 0

// Global variables for FFT
char im[128];
char data[128];
int outVal[NUM_LEVELS][num_hist];
int outHue[NUM_LEVELS];
int ii=0; // Don't modify outside main loop!
int val;
uint8_t pattern=0;
bool nextPattern = false;

// Global variables for FastLED
byte state = 1;
int index = 0;               //-LED INDEX (0 to LED_COUNT-1)
byte thisbright = 128;
byte thissat = 255;
int  thisdelay = 0;          
byte thisstep = 10;          
byte thishue = 0;

byte interrupt = 0;
 
// How many leds are in the strip?
// Be careful with the num leds. The animations that make use of the levels
// will walk right off the led array. The numbers are specific to my led
// tree. If you are using less leds or are not interested in the layer animations
// you should remove them from the loop or set USE_LEVEL_ANIMATIONS to 0
//#define NUM_LEDS 30 // LED Strip
//#define LED_TYPE    WS2812B // LED Strip
//#define COLOR_ORDER GRB // LED Strip
#define NUM_LEDS 300 //348 // Tree
#define LED_TYPE    WS2811 // Tree
#define COLOR_ORDER RGB // Tree
//int ledBin[NUM_LEDS];
 
//byte USE_LEVEL_ANIMATIONS = 1; // if 1, will auto advance past level animations
//const PROGMEM prog_uint16_t levels[NUM_LEVELS] = {58, 108, 149, 187, 224, 264, 292, 300};
//PROGMEM const uint16_t levels[NUM_LEVELS] = {58, 108, 149, 187, 224, 264, 292, 300};
 
// Data pin that led data will be written out over
#define DATA_PIN 10
// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];


/////////////////////////////////////////////////////////////////////////
// Global setup for FastLED and FFT routine
/////////////////////////////////////////////////////////////////////////
void loadSettings();
void advancePattern();

void setup() {                
  Serial.begin(9600);
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  // Load saved parameters from EEPROM
  loadSettings();
  Serial.print("Loaded pattern ");
  Serial.println(pattern);
  pattern = pattern-1; // fix for init bug
  
  // FastLED initialization
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHT);
  one_color_all(0,0,0);
  FastLED.show();
  delay(1000);
  
  int n,m;
  pinMode(pin_adc, INPUT);
  
  // Set initial values
  Serial.println("Setting initial values...\n");
  for (n=0;n<NUM_LEVELS;n++) {
    for (m=0;m<num_hist;m++) {
      outVal[n][m] = 0;
    }
  }

  
  // Bin the leds for FFT output
  Serial.println("Binning the LEDs...\n");
  /*
  int ledBin[NUM_LEDS];
  int bin = 0;
  int count = 0;
  for (n=0;n<NUM_LEDS;n++) {
    if (count>(NUM_LEDS/NUM_LEVELS)) {
      bin++;
      count = 0;
    }
    ledBin[n] = bin;
    count++;
  }
  */
  
  // Set initial hues
  int hue = 0;
  for (n=0;n<NUM_LEVELS;n++) {
    outHue[n] = hue;
    hue = hue + 255/NUM_LEVELS;
  }
  
  // Test the bins
  Serial.println("Testing the bins...\n");
  /*
  for (n=0;n<NUM_LEVELS;n++) {
    for (m=0;m<NUM_LEDS;m++) {
      if (ledBin[m]==n) {
        leds[m] = CHSV(outHue[n], thissat, thisbright);
      }
      else{
        leds[m] = CHSV(0, 0, 0);
      }
    }
    FastLED.show();
    delay(500);
  }
  */
  Serial.println("Starting!\n");
}


/////////////////////////////////////////////////////////////////////////
// LED color patterns
/////////////////////////////////////////////////////////////////////////
void one_color_all(int cred, int cgrn, int cblu)
{       //-SET ALL LEDS TO ONE COLOR
    for(int i = 0 ; i < NUM_LEDS; i++ ) {
      leds[i].setRGB( cred, cgrn, cblu).nscale8_video(thisbright);
    }
}
 
void clear_all()
{
  one_color_all(0, 0, 0);
  LEDS.show();
  delay(50);
}

void colorTest()
{
  one_color_all(0,0,0);
  FastLED.show();
  delay(500);
  one_color_all(255,0,0);
  FastLED.show();
  delay(500);
//  one_color_all(0,0,0);
//  FastLED.show();
//  delay(1000);
  one_color_all(0,255,0);
  FastLED.show();
  delay(500);
//  one_color_all(0,0,0);
//  FastLED.show();
//  delay(1000);
  one_color_all(0,0,255);
  FastLED.show();
  delay(500);
}  
 
void dotTest()
{
   // Move a single white led
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::White;
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();
      // Wait a little bit
      delay(10);
      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
}
 
void random_burst()
{
  int rndidx = random16(0, NUM_LEDS);
  int rndhue = random8(0, 255);  
  int rndbright = random8(10, thisbright);
  leds[rndidx] = CHSV(rndhue, thissat, rndbright);
  delay(random8(0, thisdelay));
}

void random_burst_white()
{
  int rndidx = random16(0, NUM_LEDS);
  int hue = 60; // set to yellow 
  int rndsat = random8(0,100);
  int rndbright = random8(10, thisbright);
  leds[rndidx] = CHSV(hue, rndsat, rndbright);
  delay(random8(0, thisdelay));
}
 
void rgb_propeller()
{
  thishue = 0;
  index++;
  int ghue = (thishue + 80) % 255;
  int bhue = (thishue + 160) % 255;
  int N3  = int(NUM_LEDS/3);
  int N6  = int(NUM_LEDS/6);  
  int N12 = int(NUM_LEDS/12);  
  for(int i = 0; i < N3; i++ ) {
    int j0 = (index + i + NUM_LEDS - N12) % NUM_LEDS;
    int j1 = (j0+N3) % NUM_LEDS;
    int j2 = (j1+N3) % NUM_LEDS;    
    leds[j0] = CHSV(thishue, thissat, thisbright);
    leds[j1] = CHSV(ghue, thissat, thisbright);
    leds[j2] = CHSV(bhue, thissat, thisbright);    
  }
}

void candycane()
{
  index++;
  int N3  = int(NUM_LEDS/3);
  int N6  = int(NUM_LEDS/6);  
  int N12 = int(NUM_LEDS/12);  
  for(int i = 0; i < N6; i++ ) {
    int j0 = (index + i + NUM_LEDS - N12) % NUM_LEDS;
    int j1 = (j0+N6) % NUM_LEDS;
    int j2 = (j1+N6) % NUM_LEDS;
    int j3 = (j2+N6) % NUM_LEDS;
    int j4 = (j3+N6) % NUM_LEDS;
    int j5 = (j4+N6) % NUM_LEDS;
    leds[j0] = CRGB(255, 255, 255).nscale8_video(thisbright*.75);
    leds[j1] = CRGB(255, 0, 0).nscale8_video(thisbright);
    leds[j2] = CRGB(255, 255, 255).nscale8_video(thisbright*.75);
    leds[j3] = CRGB(255, 0, 0).nscale8_video(thisbright);
    leds[j4] = CRGB(255, 255, 255).nscale8_video(thisbright*.75);
    leds[j5] = CRGB(255, 0, 0).nscale8_video(thisbright);
  }
}


CHSV sv_ramp( uint8_t hue, uint8_t ramp) {
  uint8_t brightness;
  uint8_t saturation;
  if( ramp < 128) {
    // fade toward black
    brightness = ramp * 2;
    brightness = dim8_video( brightness);
//    uint8_t global_brightness = FastLED.getBrightness();
//    uint8_t min_step = 256 / global_brightness;
//    brightness = qadd8( min_step * 16, brightness);
    saturation = 255;
  } else {
    // desaturate toward white
    brightness = 255;
    saturation = 255 - ((ramp - 128) * 2);
    saturation = 255 - dim8_video( 255 - saturation);
  }
  return CHSV( hue, saturation, brightness); }
 
 
void loop5( CRGB* L )
{
  uint8_t GB = FastLED.getBrightness();
  uint8_t boost = 0;
//  if( GB < 65) boost += 8;
//  if( GB < 33) boost += 8;
 
  uint8_t N = 2;
 
  static uint16_t starttheta = 0;
  starttheta += 100 / N;
 
  static uint16_t starthue16 = 0;
  starthue16 += 20 / N;
 
 
  uint16_t hue16 = starthue16;
  uint16_t theta = starttheta;
  for( int i = 0; i < NUM_LEDS; i++) {
    uint8_t frac = (sin16( theta) + 32768) / 256;
    frac = scale8(frac,160) + 32;
    theta += 3700;
 
    hue16 += 2000;
    uint8_t hue = hue16 / 256;
    L[i] = sv_ramp( hue, frac + boost);
  }
}


CHSV sv_ramp_white( uint8_t hue, uint8_t ramp) {
  uint8_t brightness;
  uint8_t saturation;
  if( ramp < 128) {
    // fade toward black
    brightness = ramp * 2;
    brightness = dim8_video( brightness);
//    uint8_t global_brightness = FastLED.getBrightness();
//    uint8_t min_step = 256 / global_brightness;
//    brightness = qadd8( min_step * 16, brightness);
    saturation = 175;
  } else {
    // desaturate toward white
    brightness = 255;
    saturation = 255 - ((ramp - 128) * 2);
    saturation = scale8(255 - dim8_video( 255 - saturation),175);
  }
  return CHSV( hue, saturation, brightness); }
  
   

void treeWhiteTwinkle( CRGB* L )
{
  uint8_t GB = FastLED.getBrightness();
  uint8_t boost = 0;
//  if( GB < 65) boost += 8;
//  if( GB < 33) boost += 8;
 
  uint8_t N = 2;
 
  static uint16_t starttheta = 0;
  starttheta += 100 / N;
 
//  static uint16_t starthue16 = 0;
//  starthue16 += 20 / N;
 
 
//  uint16_t hue16 = starthue16;
  uint16_t theta = starttheta;
  for( int i = 0; i < NUM_LEDS; i++) {
    uint8_t frac = (sin16( theta) + 32768) / 256;
    frac = scale8(frac,160) + 32;
    theta += 3700;
 
    //hue16 += 2000;
    uint8_t hue = 60; //set to yellow! //hue16 / 256;
    L[i] = sv_ramp_white( hue, frac + boost);
  }
}

void rainbow_fade()
{
    thishue++;
    if (thishue > 255) {thishue = 0;}
    for(int idex = 0 ; idex < NUM_LEDS; idex++ ) {
      leds[idex] = CHSV(thishue, thissat, thisbright);
    }
}


/////////////////////////////////////////////////////////////////////////
// Main loop
/////////////////////////////////////////////////////////////////////////
boolean INIT = 0;
/*
byte alt = 0;
boolean doRandom = 0;
boolean stateInc = 0;
int randState = 1;
*/
unsigned long delayTimer = 0;
// Second vector of led's for transition
//CHSV hsv_target[NUM_LEDS];

void loop(){
 int n;
 //int b;
 int sum;
 int outMult[NUM_LEVELS] = {10,15,25,30,35,35};
 float alpha[NUM_LEVELS] = {0.3,0.25,0.22,0.22,0.22,0.22};
 //int adjdelay = thisdelay;
 char buf[35];
 
  if (!INIT) {
    //Light the tree
    dotTest();
    //colorTest();
    clear_all();
    INIT = 1;
  }
  
  // Default pattern
  //rainbow_fade();
  //adjdelay = thisdelay;
  
   if (ii < 128) {
   // Take data
   val = analogRead(pin_adc);
   // Signal conditioned to 2.5 V +/- 2.5 V 
   // -- (analog reads in 512 +/- 512) -- use full range of 8-bit FFT (-128 to 128)
   data[ii] = (val-512)/4;
   im[ii] = 0;
   ii++;   
   
   }
   else {
   // Do the in-place, fixed-point FFT
   fix_fft(data,im,7,0);
   
   // I am only interested in the absolute value of the transformation
   for (int i=0; i< 64;i++){
      data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
   }
   
   // Keep an exponential moving average for each output bin
   for (n=0;n<NUM_LEVELS;n++) {
     sum = 0;
     for (int i=1;i<=10;i++) {
       if (data[n*10+i]>sum) {
         sum = data[n*10+i];
       }
     }
              
     // Avoid flickering
     if (sum<3) {
       sum = 0;
     }
     //sum = sum*outMult[n];
     sum = sum*outMult[n];
     
     // Update the average
     outVal[n][1] = outVal[n][0];
     outVal[n][0] += alpha[n]*(sum - outVal[n][0]);
  
     // Clip at max brightness
     if (outVal[n][0]>MAX_BRIGHT) {
       outVal[n][0] = MAX_BRIGHT;
     }
   }
   

   if (outVal[0][0]>0) {
      delayTimer = millis();
      // Print to monitor
      sprintf(buf,"out %i,%i,%i,%i,%i,%i",outVal[0][0],outVal[1][0],outVal[2][0],outVal[3][0],outVal[4][0],outVal[5][0]);
      Serial.println(buf);
   }

   if ((millis()-delayTimer)>10000) {
      //Serial.println(pattern);
      if (nextPattern) {
        nextPattern = false;
        advancePattern(); // Saves state to EEPROM
        Serial.print("Switching to pattern ");
        Serial.println(pattern);
      }
      // more than ten seconds of silence, show white LEDs
      if (pattern==0) {
        treeWhiteTwinkle(leds);
      } else if (pattern==1) {
        loop5(leds);
      } else if (pattern==2) {
        random_burst();
      } 
      /*else if (pattern==3) {
        rainbow_fade();
      }
      */ 
      else {
        // Reset to the first pattern
        pattern = 0;
      }
   }
   else {
     nextPattern = true;
     // Next step -- update color and brightness
     //thishue++;
     //if (thishue > 255) {thishue = 0;}
     for (n=0;n<NUM_LEVELS;n++) {
       outHue[n]++;
       if (outHue[n] > 255) {outHue[n] = 0;}
     }
     int bin=0;
     int count=0;
     for(int idex = 0 ; idex < NUM_LEDS; idex++ ) {
       // Get the bin
       if (count>(NUM_LEDS/NUM_LEVELS)) {
         bin++;
         count=0;
       }
       // Set the brightness from the FFT
       leds[idex] = CHSV(outHue[bin], thissat, outVal[bin][0]);
       count++;
       
       // Can't do this with >200 LEDs; ledBin is too big for Arduino dynamic memory
       //leds[idex] = CHSV(outHue[ledBin[idex]], thissat, outVal[ledBin[idex]][0]);
     }
   }

   FastLED.show(); 
   
   ii=0;
}

}


void loadSettings() {
  pattern = EEPROM.read(0);
}

void advancePattern() {
  pattern = pattern+1 > 2 ? 0 : pattern+1;
  EEPROM.write(0, pattern);
}
