
#include "Config.h"

#define READ EEPROM_readAnything
#define WRITE EEPROM_writeAnything

void Config::applyBulk(bool read){
  int i = 1;
  i += read ? READ(i, PPR) : WRITE(i, PPR);
  i += read ? READ(i, RPMStep) : WRITE(i, RPMStep);
  i += read ? READ(i, RPMBuffer) : WRITE(i, RPMBuffer);
  i += read ? READ(i, RPMAnimation) : WRITE(i, RPMAnimation);
  i += read ? READ(i, RPMMeasureMode) : WRITE(i, RPMMeasureMode);
  i += read ? READ(i, PixelBrightness) : WRITE(i, PixelBrightness);
  i += read ? READ(i, DisplayBrightness) : WRITE(i, DisplayBrightness);
  i += read ? READ(i, CLow) : WRITE(i, CLow);
  i += read ? READ(i, CPart1) : WRITE(i, CPart1);
  i += read ? READ(i, CPart2) : WRITE(i, CPart2);
  i += read ? READ(i, CPart3) : WRITE(i, CPart3);
  i += read ? READ(i, CFlash1) : WRITE(i, CFlash1);
  i += read ? READ(i, CFlash2) : WRITE(i, CFlash2);
  i += read ? READ(i, RPMStationary) : WRITE(i, RPMStationary);
  i += read ? READ(i, RPMLow) : WRITE(i, RPMLow);
  i += read ? READ(i, RPMActivation) : WRITE(i, RPMActivation);
  i += read ? READ(i, RPMShift) : WRITE(i, RPMShift);
}

void Config::load(){
  uint8_t magicByte;
  EEPROM_readAnything(0, magicByte);
  
  //In case we didn't find our magic byte, use the default values.
  if(magicByte != EEPROM_MAGIC_BYTE){
    PPR = DEFAULT_PPR;
    RPMStep = DEFAULT_RPM_STEP;
    RPMBuffer = DEFAULT_RPM_BUFFER;
    RPMAnimation = DEFAULT_RPM_ANIMATION;
    RPMMeasureMode = DEFAULT_RPM_MEASUREMODE;
    PixelBrightness = DEFAULT_PIXEL_BRIGHTNESS;
    DisplayBrightness = DEFAULT_DISPLAY_BRIGHTNESS;
    CLow = DEFAULT_CLOW;
    CPart1 = DEFAULT_CPART1;
    CPart2 = DEFAULT_CPART2;
    CPart3 = DEFAULT_CPART3;
    CFlash1 = DEFAULT_CFLASH1;
    CFlash2 = DEFAULT_CFLASH2;
    RPMStationary = DEFAULT_RPM_STATIONARY;
    RPMLow = DEFAULT_RPM_LOW;
    RPMActivation = DEFAULT_RPM_ACTIVATION;
    RPMShift = DEFAULT_RPM_SHIFT;
    return;
  }
  
  //Otherwise, do a normal read of all values from EEPROM.
  applyBulk(true);
  
  //Do a few sanity checks.
  if(PPR > 12 || RPMBuffer > 1 || RPMMeasureMode > 1){
    //Unfortunately we've gone full EEPtard. Never go full EEPtard.
    reset();
  }
  
}

void Config::save(){
  EEPROM_writeAnything(0, EEPROM_MAGIC_BYTE);
  applyBulk(false);
}

void Config::reset(){
  //This will trigger all the defaults, which we will save.
  EEPROM_writeAnything(0, 0xFF);
  load();
  save();
}
