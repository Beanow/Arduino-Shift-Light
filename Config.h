
#ifndef Config_h
#define Config_h

#include <EEPROMAnything.h>
#include "_defines.h"
#include "Profile.h"

#define EEPROM_MAGIC_BYTE 0x55

class Config {
  
  public:
    
    void setCurrentProfile(uint8_t profileIndex);
    void setCurrentProfile(Profile profile);
    
    bool
      RPMBuffer;
    
    int8_t
      RPMMeasureMode;
    
    uint8_t
      PPR,
      RPMStep,
      ProfileCount,
      PixelBrightness,
      DisplayBrightness;
    
    uint16_t
      RPMStationary;
    
    Profile
      *Profiles = NULL,
      *CurrentProfile;
    
    void
      load(),
      save(),
      reset();
  
  private:
    void applyBulk(bool read);
    
    uint8_t currentProfileIndex;
    
};

#endif
