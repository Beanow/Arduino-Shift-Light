
#include "PixelAnimator.h"

static CRGB pixels[NUMPIXELS];

#define EXP_BRIGHT_STEP 0.2857
#define EXP_BRIGHT_OFFSET 4.0
#define EXP_BRIGHT_POW 4
int expBrightness(double in){
  return (int)pow(EXP_BRIGHT_STEP*(in+EXP_BRIGHT_OFFSET), EXP_BRIGHT_POW);
}

PixelAnimator::PixelAnimator(Config* config){
  CONFIG = config;
}

void PixelAnimator::setup(){
  FastLED.addLeds<PIXELTYPE, PIXELPIN, GRB>(pixels, NUMPIXELS);
  setBrightness(CONFIG->PixelBrightness);
  updateColors();
}

void PixelAnimator::setBrightness(uint8_t brightness){
  targetBrightness = expBrightness(brightness);
}

void PixelAnimator::updateColors(){
  CLow = ColorPicker::at(CONFIG->CLow);
  CPart1 = ColorPicker::at(CONFIG->CPart1);
  CPart2 = ColorPicker::at(CONFIG->CPart2);
  CPart3 = ColorPicker::at(CONFIG->CPart3);
  CFlash1 = ColorPicker::at(CONFIG->CFlash1);
  CFlash2 = ColorPicker::at(CONFIG->CFlash2, true);
}

void PixelAnimator::show(){
  FastLED.setBrightness(calculate_max_brightness_for_power_mW(targetBrightness, MAX_POWER));
  FastLED.show();
}

void PixelAnimator::setRPM(uint16_t rpm){
  int litPixels, blackPixels, midPoint;
  bool shift = false, blank = false, low = false;
  
  if(rpm >= CONFIG->RPMShift){ shift = true; }
  else if(rpm <= CONFIG->RPMLow && rpm > CONFIG->RPMStationary){ low = true; }
  else if(rpm <= CONFIG->RPMActivation){ blank = true; }
  else{
    float fillRange, fillOffset;
    fillOffset = rpm - CONFIG->RPMActivation;
    fillRange = CONFIG->RPMShift - CONFIG->RPMActivation;
    litPixels = fillOffset / fillRange * NUMPIXELS;
  }
  
  // Do shift flashing.
  if(shift){
    CRGB color = (millis() / ANIM_TURBO % 2) ? CFlash1 : CFlash2;
    setFill(color);
  }
  
  // Do low RPM indication.
  else if(low){
    CRGB color = (millis() / ANIM_FAST % 2) ? CLow : CRGB::Black;
    setFill(color);
  }
  
  // When we show nothing.
  else if(blank){
    for(int i=0; i<NUMPIXELS; i++) pixels[i] = CRGB::Black;
  }
  
  // When not shifting yet.
  else{
    
    // Pick a filling style.
    switch(CONFIG->RPMAnimation){
      
      default:
      case RPMAnimation::LeftToRight:
        for(int i = 0; i < NUMPIXELS; i++){
          pixels[i] = (litPixels >= i ? colorFor(i) : CRGB::Black);
        }
        break;
      
      case RPMAnimation::RightToLeft:
        blackPixels = NUMPIXELS - litPixels - 1;
        for(int i = 0; i < NUMPIXELS; i++){
          pixels[i] = (blackPixels <= i ? colorFor(NUMPIXELS-1-i) : CRGB::Black);
        }
        break;
      
      case RPMAnimation::Outward:
        midPoint = ceil(NUMPIXELS / 2.0);
        blackPixels = midPoint - round((litPixels+1) / 2.0);
        for(int i = 0; i < midPoint; i++){
          pixels[i] =             (blackPixels <= i ? colorFor(midPoint-1-i, true) : CRGB::Black);
          pixels[NUMPIXELS-1-i] = (blackPixels <= i ? colorFor(midPoint-1-i, true) : CRGB::Black);
        }
        break;
      
      case RPMAnimation::Inward:
        midPoint = ceil(NUMPIXELS / 2.0);
        litPixels = litPixels / 2;
        for(int i = 0; i < midPoint; i++){
          pixels[i] =             (litPixels >= i ? colorFor(i, true) : CRGB::Black);
          pixels[NUMPIXELS-1-i] = (litPixels >= i ? colorFor(i, true) : CRGB::Black);
        }
        break;
      
      case RPMAnimation::Full:
        uint8_t progress = 3.0/NUMPIXELS*litPixels;
        CRGB color;
        switch (progress) {
            case 0: color = CPart1; break;
            case 1: color = CPart2; break;
            case 2: color = CPart3; break;
        }
        setFill(color);
        // float newBright = (float)litPixels / NUMPIXELS * (CONFIG->PixelBrightness-2.0) + 2.0;
        // setBrightness(newBright);
        break;
      
    }
    
  }
  
}

void PixelAnimator::setFill(CRGB color){
  for(int i = 0; i < NUMPIXELS; i++){
    pixels[i] = color;
  }
}

void PixelAnimator::setEdges(CRGB color){
  pixels[0] = color;
  pixels[NUMPIXELS-1] = color;
}

void PixelAnimator::showBlockingRunlight(CRGB color, uint16_t time){
  uint8_t length = 5;
  uint8_t stepDelay = time / (NUMPIXELS+length);
  setFill(CRGB::Black);
  for (int i = 0; i < NUMPIXELS+length; ++i){
    if(i < NUMPIXELS) pixels[i]        = color;
    if(i-length >= 0) pixels[i-length] = CRGB::Black;
    FastLED.delay(stepDelay);
  }
}

CRGB PixelAnimator::colorFor(uint8_t index, bool halved){
  if(halved){
    
    if(index < (NUMPIXELS/4))
      return CPart1;
    
    else if(index <= (NUMPIXELS*3/8))
      return CPart2;
    
    else
      return CPart3;
    
  } else {
    
    if(index <= (NUMPIXELS/2))
      return CPart1;
    
    else if(index <= (NUMPIXELS*3/4))
      return CPart2;
    
    else
      return CPart3;
    
  }
}
