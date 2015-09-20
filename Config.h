
#ifndef Config_h
#define Config_h

#include <EEPROMAnything.h>
#include "_defines.h"

#define EEPROM_MAGIC_BYTE 0x55

class Config {
  
  public:
    
    bool
      RPMBuffer;
    
    int8_t
      RPMMeasureMode;
    
    uint8_t
      PPR,
      RPMStep,
      RPMAnimation,
      PixelBrightness,
      DisplayBrightness,
      CLow,
      CPart1,
      CPart2,
      CPart3,
      CFlash1,
      CFlash2;
    
    uint16_t
      RPMStationary,
      RPMLow,
      RPMActivation,
      RPMShift;
    
    void
      load(),
      save(),
      reset();
  
  private:
    void applyBulk(bool read);
  
};

#endif
