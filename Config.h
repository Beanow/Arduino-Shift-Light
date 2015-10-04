
#ifndef Config_h
#define Config_h

#include <EEPROMAnything.h>
#include "_defines.h"
#include "Profile.h"

#define EEPROM_MAGIC_BYTE 0x56

class Config {
  
  public:
    Profile* newProfile();
    uint8_t getProfileCount();
    uint8_t getCurrentProfileIndex();
    void deleteProfile(Profile* profile);
    void setCurrentProfile(uint8_t profileIndex);
    
    bool
      RPMBuffer;
    
    int8_t
      RPMMeasureMode;
    
    uint8_t
      PPR,
      RPMStep,
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
    
    uint8_t
      profileCount,
      currentProfileIndex;
    
};

#endif
