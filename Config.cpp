
#include "Config.h"

#define READ EEPROM_readAnything
#define WRITE EEPROM_writeAnything

void Config::applyBulk(bool read){
  int i = 1;
  i += read ? READ(i, PPR) : WRITE(i, PPR);
  i += read ? READ(i, RPMStep) : WRITE(i, RPMStep);
  i += read ? READ(i, RPMBuffer) : WRITE(i, RPMBuffer);
  i += read ? READ(i, RPMMeasureMode) : WRITE(i, RPMMeasureMode);
  i += read ? READ(i, PixelBrightness) : WRITE(i, PixelBrightness);
  i += read ? READ(i, DisplayBrightness) : WRITE(i, DisplayBrightness);
  i += read ? READ(i, RPMStationary) : WRITE(i, RPMStationary);
  
  //Doing the profile parts.
  uint8_t previousProfileCount = profileCount;
  i += read ? READ(i, profileCount) : WRITE(i, profileCount);
  i += read ? READ(i, currentProfileIndex) : WRITE(i, currentProfileIndex);
  
  //When we're reading, we need to manage our profiles.
  if(read){
    
    //Rebuild the array if the profile count is different.
    if(profileCount != previousProfileCount){
      if (Profiles) delete[] Profiles;
      Profiles = new Profile[profileCount];
    }
    
    //Update our pointer to the right profile.
    setCurrentProfile(currentProfileIndex);
    
  }
  
  //Either way, run the desired operation.
  for (int pi = 0; pi < profileCount; ++pi){
    i += read ? Profiles[pi].read(i) : Profiles[pi].write(i);
  }
}

void Config::load(){
  uint8_t magicByte;
  EEPROM_readAnything(0, magicByte);
  
  //In case we didn't find our magic byte, use the default values.
  if(magicByte != EEPROM_MAGIC_BYTE){
    PPR = DEFAULT_PPR;
    RPMStep = DEFAULT_RPM_STEP;
    RPMBuffer = DEFAULT_RPM_BUFFER;
    RPMMeasureMode = DEFAULT_RPM_MEASUREMODE;
    PixelBrightness = DEFAULT_PIXEL_BRIGHTNESS;
    DisplayBrightness = DEFAULT_DISPLAY_BRIGHTNESS;
    RPMStationary = DEFAULT_RPM_STATIONARY;
    profileCount = 1;
    if (Profiles) delete[] Profiles;
    Profiles = new Profile[1];
    setCurrentProfile(0);
    return;
  }
  
  //Otherwise, do a normal read of all values from EEPROM.
  applyBulk(true);
  
  //Do a few sanity checks.
  if(PPR > 12 || RPMBuffer > 1 || RPMMeasureMode > 1 || profileCount > 10){
    //Unfortunately we've gone full EEPtard. Never go full EEPtard.
    reset();
  }
  
}

uint8_t Config::getProfileCount(){ return profileCount; }
uint8_t Config::getCurrentProfileIndex(){ return currentProfileIndex; }

void Config::setCurrentProfile(Profile profile){
  for (int i = 0; i < profileCount; ++i){
    if(&profile == &(Profiles[i])){
      currentProfileIndex = i;
      CurrentProfile = &(Profiles[i]);
      return;
    }
  }
}

void Config::setCurrentProfile(uint8_t profileIndex){
  if(profileIndex >= profileCount) return;
  currentProfileIndex = profileIndex;
  CurrentProfile = &(Profiles[profileIndex]);
}

Profile* Config::newProfile(){
  Profile* oldProfiles = Profiles;
  Profiles = new Profile[++profileCount];
  for (int i = 0; i < profileCount-1; ++i){
    memcpy(&(Profiles[i]), &(oldProfiles[i]), sizeof(Profile));
  }
  delete[] oldProfiles;
  setCurrentProfile(currentProfileIndex);
  return &(Profiles[profileCount-1]);
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
