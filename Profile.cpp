#include "Profile.h"

#define READ EEPROM_readAnything
#define WRITE EEPROM_writeAnything

Profile::Profile(){
    RPMAnimation = DEFAULT_RPM_ANIMATION;
    CLow = DEFAULT_CLOW;
    CPart1 = DEFAULT_CPART1;
    CPart2 = DEFAULT_CPART2;
    CPart3 = DEFAULT_CPART3;
    CFlash1 = DEFAULT_CFLASH1;
    CFlash2 = DEFAULT_CFLASH2;
    RPMLow = DEFAULT_RPM_LOW;
    RPMActivation = DEFAULT_RPM_ACTIVATION;
    RPMShift = DEFAULT_RPM_SHIFT;
}

int Profile::applyBulk(bool read, int i){
  i += read ? READ(i, CLow) : WRITE(i, CLow);
  i += read ? READ(i, CPart1) : WRITE(i, CPart1);
  i += read ? READ(i, CPart2) : WRITE(i, CPart2);
  i += read ? READ(i, CPart3) : WRITE(i, CPart3);
  i += read ? READ(i, CFlash1) : WRITE(i, CFlash1);
  i += read ? READ(i, CFlash2) : WRITE(i, CFlash2);
  i += read ? READ(i, RPMAnimation) : WRITE(i, RPMAnimation);
  i += read ? READ(i, RPMLow) : WRITE(i, RPMLow);
  i += read ? READ(i, RPMActivation) : WRITE(i, RPMActivation);
  i += read ? READ(i, RPMShift) : WRITE(i, RPMShift);
  return i;
}

int Profile::read(int startingIndex){
  return applyBulk(true, startingIndex);
}

int Profile::write(int startingIndex){
  return applyBulk(false, startingIndex);
}
