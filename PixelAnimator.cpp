
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
  passedLow = false;
  CSegmentsFull = new CRGB*[NUMPIXELS];
  int halfMax = ceil(NUMPIXELS/2.0);
  CSegmentsHalved = new CRGB*[halfMax];
  FastLED.addLeds<PIXELTYPE, PIXELPIN, GRB>(pixels, NUMPIXELS);
  setBrightness(CONFIG->PixelBrightness);
  updateColors();
}

void PixelAnimator::setBrightness(uint8_t brightness){
  targetBrightness = expBrightness(brightness);
}

void PixelAnimator::updateColors(){
  CLow = ColorPicker::at(CONFIG->CurrentProfile->CLow);
  CPart1 = ColorPicker::at(CONFIG->CurrentProfile->CPart1);
  CPart2 = ColorPicker::at(CONFIG->CurrentProfile->CPart2);
  CPart3 = ColorPicker::at(CONFIG->CurrentProfile->CPart3);
  CFlash1 = ColorPicker::at(CONFIG->CurrentProfile->CFlash1);
  CFlash2 = ColorPicker::at(CONFIG->CurrentProfile->CFlash2, true);
  int i = 0;
  for (; i < DEFAULT_SEG1 && i < NUMPIXELS; ++i) CSegmentsFull[i] = &CPart1;
  for (; i < DEFAULT_SEG1+DEFAULT_SEG2 && i < NUMPIXELS; ++i) CSegmentsFull[i] = &CPart2;
  for (; i < NUMPIXELS; ++i) CSegmentsFull[i] = &CPart3;
  i = 0;
  int halfMax = ceil(NUMPIXELS/2.0);
  float halvingFactor = halfMax/(float)NUMPIXELS;
  int halfSeg1 = DEFAULT_SEG1*halvingFactor;
  int halfSeg2 = DEFAULT_SEG2*halvingFactor;
  //We want to show all segments at at least 1 pixel size.
  if(halfSeg1+halfSeg2 == halfMax){
    if(halfSeg2 > 1) halfSeg2--;
    else halfSeg1--;
  }
  for (; i < halfSeg1 && i < halfMax; ++i) CSegmentsHalved[i] = &CPart1;
  for (; i < halfSeg1+halfSeg2 && i < halfMax; ++i) CSegmentsHalved[i] = &CPart2;
  for (; i < halfMax; ++i) CSegmentsHalved[i] = &CPart3;
}

void PixelAnimator::show(){
  FastLED.setBrightness(calculate_max_brightness_for_power_mW(targetBrightness, MAX_POWER));
  FastLED.show();
}

void PixelAnimator::setRPM(uint16_t rpm){
  int litPixels, blackPixels, midPoint;
  bool shift = false, blank = false, low = false;
  
  if(rpm >= CONFIG->CurrentProfile->RPMShift){ shift = true; passedLow = true; }
  else if(passedLow && rpm <= CONFIG->CurrentProfile->RPMLow && rpm > CONFIG->RPMStationary){ low = true; }
  else if(rpm <= CONFIG->CurrentProfile->RPMActivation){
    blank = true;
    if(rpm < CONFIG->RPMStationary)
      passedLow = false;
    else if (rpm > CONFIG->CurrentProfile->RPMLow)
      passedLow = true;
  }
  else{
    passedLow = true;
    float fillRange, fillOffset;
    fillOffset = rpm - CONFIG->CurrentProfile->RPMActivation;
    fillRange = CONFIG->CurrentProfile->RPMShift - CONFIG->CurrentProfile->RPMActivation;
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
    setFill(CRGB::Black);
  }
  
  // When not shifting yet.
  else{
    
    // Pick a filling style.
    switch(CONFIG->CurrentProfile->RPMAnimation){
      
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
    return *CSegmentsHalved[index];
  } else {
    return *CSegmentsFull[index];
  }
}
