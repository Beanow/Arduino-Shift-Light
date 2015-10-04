#ifndef Profile_h
#define Profile_h

#include <EEPROMAnything.h>
#include "_defines.h"

class Profile {
  
  public:
    Profile();
    int read(int startingIndex);
    int write(int startingIndex);
    
    uint8_t
      RPMAnimation,
      CLow,
      CPart1,
      CPart2,
      CPart3,
      CFlash1,
      CFlash2;
    
    uint16_t
      RPMLow,
      RPMActivation,
      RPMShift;
  
  private:
    int applyBulk(bool read, int startingIndex);
    
};

#endif
