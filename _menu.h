
#ifndef _menu_h
#define _menu_h

#include "MenuItem.h"
#include <FastLED.h>
#include "_defines.h"
#include "_menuHelpers.h"

//These need to stay at the top, and you can't use 0.
//Their numbers also define the menu ordering.
#define _Home_ 1
#define _QuickBrightness_ 2
#define _ProfileSwitcher_ 3

#define _MainMenu_ 10 //This is always the first main menu item.
#define _ProfileListing_ 11
#define _EditLCDBrightness_ 12
#define _EditColorSegments_ 13
#define _EditTime_ 14
#define _EditRPMStep_ 15
#define _EditStationaryRPM_ 16
#define _EditPPR_ 17
#define _EditRPMBuffer_ 18
#define _EditRPMMeasureMode_ 19
#define _Reset_ 20
#define LAST_MAIN_MENU_ITEM _Reset_ //Constraint the main menu.

#define _EditProfileLowRPM_ 31
#define _EditProfileActivationRPM_ 32
#define _EditProfileShiftRPM_ 33
#define _EditProfileRPMAnimation_ 34
#define _EditProfileColors_ 35
#define _DeleteProfile_ 36
#define FIRST_PROFILE_MENU_ITEM _EditProfileLowRPM_ //Constraint the profile menu.
#define LAST_PROFILE_MENU_ITEM _DeleteProfile_ //Constraint the profile menu.

#define _EditColorLow_ 40
#define _EditColorPart1_ 41
#define _EditColorPart2_ 42
#define _EditColorPart3_ 43
#define _EditColorFlash1_ 44
#define _EditColorFlash2_ 45
#define FIRST_COLOR_MENU_ITEM _EditColorLow_ //Constraint the color menu.
#define LAST_COLOR_MENU_ITEM _EditColorFlash2_ //Constraint the color menu.

/* ==  Home == */
class HomeMenuItem : public MenuItem {
  
  bool showRPM, autoOff, manualOff;
  uint32_t zeroStart;
  
  void onEnter(){
    zeroStart = 0;
    showRPM = true;
    autoOff = rpm->getRPM() == 0;
    display->setBrightness(autoOff ? DISPLAY_OFF_BRIGHTNESS : CONFIG->DisplayBrightness);
    animator->setBrightness(CONFIG->PixelBrightness);
    animator->showBlockingRunlight(CRGB::Green);
  }
  
  void onUpdate(){
    
    //RPM on the LCD display.
    uint16_t m_rpm = rpm->getRPM();
    
    //Track zero, to switch off.
    if(m_rpm == 0){
      if(!autoOff){
        //If we were not at 0 before.
        if(!zeroStart){
          zeroStart = millis();
        }
        //Check if we timed out yet.
        else if(millis() - zeroStart >= POWER_OFF_TIMEOUT) {
          autoOff = true;
          zeroStart = 0;
          if(!manualOff){
            animator->showBlockingRunlight(CRGB::Red);
            display->setBrightness(DISPLAY_OFF_BRIGHTNESS);
          }
        }
      }
    } else {
      //Reset the counter, since we're not at zero.
      zeroStart = 0;
      if(autoOff){
        autoOff = false;
        if(!manualOff){
          display->setColon(false);
          animator->showBlockingRunlight(CRGB::Green);
          display->setBrightness(CONFIG->DisplayBrightness);
        }
      }
    }
    
    //Either show the clock or RPM on the display.
    if(showRPM && !(autoOff || manualOff)){
      
      //Floor it to our step value.
      if(CONFIG->RPMStep > 1){
        m_rpm = m_rpm / CONFIG->RPMStep * CONFIG->RPMStep;
      }
      display->showNumberDec(m_rpm);
      
    }
    
    else {
      
      //Clock on the LCD display.
      Time t = rtcClock->time();
      display->setColon(t.sec % 2);
      display->showNumberDec(t.hr*100+t.min);
      
    }
    
    if(autoOff || manualOff){
      delay(ADDITIONAL_INTERVAL);
    }else{
      //RPM as displayed by the RGB pixels.
      animator->setRPM(rpm->getRPM());
      animator->show();
    }
    
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    if(manualOff && event != HoldUp){
      manualOff = false;
      display->setColon(false);
      animator->showBlockingRunlight(CRGB::Green);
      if(!autoOff){
        display->setBrightness(CONFIG->DisplayBrightness);
      }
      return;
    }
    switch (event) {
    
    case HoldUp:
      if(!manualOff){
        manualOff = true;
        animator->showBlockingRunlight(CRGB::Red);
        display->setBrightness(DISPLAY_OFF_BRIGHTNESS);
      }
      break;
    
    case Up:
      MenuItem::enter(_QuickBrightness_);
      break;
    
    case Left:
      MenuItem::enter(_ProfileSwitcher_);
      break;
    
    case Right:
      MenuItem::enter(_MainMenu_);
      animator->showBlockingRunlight(CRGB::Green);
      break;
    
    case Down:
      showRPM = !showRPM;
      display->setColon(false);
      break;
    
  }}
  
};

/* ==  QuickBrightness == */
class QuickBrightnessMenuItem : public MenuItem {
  
  void onEnter(){
    animator->setFill(CRGB::Blue);
  }
  
  void onLeave(){
    CONFIG->save();
  }
  
  void onUpdate(){
    //Brightness on the LCD display.
    display->showNumberDec(CONFIG->PixelBrightness);
    animator->setBrightness(CONFIG->PixelBrightness);
    animator->show();
  }
  
  void onButtonEvent(buttonSetEvent_t event){ switch (event) {
    
    case Up:
      CONFIG->PixelBrightness = min(CONFIG->PixelBrightness+1, 10);
      animator->setBrightness(CONFIG->PixelBrightness);
      break;
    
    case Down:
      CONFIG->PixelBrightness = max(CONFIG->PixelBrightness-1, 1);
      animator->setBrightness(CONFIG->PixelBrightness);
      break;
    
    case Left:
    case Right:
      MenuItem::enter(_Home_);
      break;
    
  }}
  
};

/* ==  ProfileSwitcher == */
class ProfileSwitcherMenuItem : public MenuItem {
  
  uint8_t profileIndex;
  uint8_t displaySegments[4];
  uint8_t displaySegmentsPrefix[2] = {
    SEGMENT_P,
    SEGMENT_r
  };
  
  void onEnter(){
    profileIndex = CONFIG->getCurrentProfileIndex();
    displaySegments[0] = displaySegmentsPrefix[0];
    displaySegments[1] = displaySegmentsPrefix[1];
    displaySegments[2] = SEGMENT_BLANK;
    displaySegments[3] = display->encodeDigit(profileIndex+1);
    display->setSegments(displaySegments);
    animator->show();
  }
  
  void onButtonEvent(buttonSetEvent_t event){ switch (event) {
    
    case Up:
      profileIndex--;
      //Overflowed past 0, ignore it.
      if(profileIndex == 255)
        profileIndex = 0;
      else{
        displaySegments[2] = SEGMENT_BLANK;
        displaySegments[3] = display->encodeDigit(profileIndex+1);
        display->setSegments(displaySegments);
      }
      break;
    
    case Down:
      profileIndex++;
      //Went past profile count, ignore it.
      if(profileIndex >= CONFIG->getProfileCount())
        profileIndex = CONFIG->getProfileCount()-1;
      else{
        displaySegments[2] = SEGMENT_BLANK;
        displaySegments[3] = display->encodeDigit(profileIndex+1);
        display->setSegments(displaySegments);
      }
      break;
    
    case Right:
    case Left:
      CONFIG->setCurrentProfile(profileIndex);
      animator->updateColors();
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  MainMenu == */
class MainMenuMenuItem : public MenuItem {
  
  uint8_t displaySegments[4] = {
    0b00111001, //  _ _ _ _
    0b00001001, // |       |
    0b00001001, // |_ _ _ _|
    0b00001111  //
  };
  
  void onEnter(){
    display->setSegments(displaySegments);
  }
  
  void onButtonEvent(buttonSetEvent_t event){ switch (event) {
    
    case Up: MenuItem::mainMenuPrev(_MainMenu_); break;
    case Down: MenuItem::mainMenuNext(_MainMenu_); break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  ProfileListing == */
class ProfileListingMenuItem : public MenuItem {
  
  uint8_t profileIndex;
  uint8_t displaySegments[4];
  uint8_t displaySegmentsPrefix[2] = {
    SEGMENT_P,
    SEGMENT_r
  };
  
  void onEnter(){
    //If we came from the menu item below us, behave differently.
    bool reverse = prevMenuItemIndex == _ProfileListing_+1;
    bool maxed = CONFIG->getProfileCount() == MAX_PROFILES;
    profileIndex = reverse ? CONFIG->getProfileCount()-(maxed?1:0) : 0;
    displaySegments[0] = displaySegmentsPrefix[0];
    displaySegments[1] = displaySegmentsPrefix[1];
    displaySegments[2] = reverse && !maxed ? SEGMENT_DASH : SEGMENT_BLANK;
    displaySegments[3] = reverse && !maxed ? SEGMENT_DASH : display->encodeDigit(profileIndex+1);
    display->setSegments(displaySegments);
  }
  
  void onButtonEvent(buttonSetEvent_t event){ switch (event) {
    
    case Up:
      profileIndex--;
      if(profileIndex == 255) //Overflowed past 0
        MenuItem::mainMenuPrev(_ProfileListing_);
      else{
        displaySegments[2] = SEGMENT_BLANK;
        displaySegments[3] = display->encodeDigit(profileIndex+1);
        display->setSegments(displaySegments);
      }
      break;
    
    case Down:
      profileIndex++;
      if(profileIndex > CONFIG->getProfileCount() || profileIndex == MAX_PROFILES)
        MenuItem::mainMenuNext(_ProfileListing_);
      else if(profileIndex == CONFIG->getProfileCount()){
        displaySegments[2] = SEGMENT_DASH;
        displaySegments[3] = SEGMENT_DASH;
        display->setSegments(displaySegments);
      }
      else{
        displaySegments[2] = SEGMENT_BLANK;
        displaySegments[3] = display->encodeDigit(profileIndex+1);
        display->setSegments(displaySegments);
      }
      break;
    
    case Right:
      if(profileIndex == CONFIG->getProfileCount()){
        PROFILE = CONFIG->newProfile();
        MenuItem::enter(FIRST_PROFILE_MENU_ITEM);
      }
      else{
        PROFILE = &(CONFIG->Profiles[profileIndex]);
        MenuItem::enter(FIRST_PROFILE_MENU_ITEM);
      }
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      break;
    
  }}
  
};

/* ==  EditPPR == */
class EditPPRMenuItem : public EditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_P,
    SEGMENT_P,
    SEGMENT_r
  };
  
  void onLeave(){
    if(didEdit) rpm->setPulsesPerRevolution(CONFIG->PPR);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->PPR);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up:
      if(!editing) MenuItem::mainMenuPrev(_EditPPR_);
      else CONFIG->PPR = constrain(CONFIG->PPR+1, 1, 12);
      break;
    
    case Down:
      if(!editing) MenuItem::mainMenuNext(_EditPPR_);
      else CONFIG->PPR = constrain(CONFIG->PPR-1, 1, 12);
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditLCDBrightness == */
class EditLCDBrightnessMenuItem : public EditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_L,
    SEGMENT_c,
    SEGMENT_d
  };
  
  void onUpdate(){
    if(editing){
      display->setBrightness(CONFIG->DisplayBrightness);
      display->showNumberDec(CONFIG->DisplayBrightness);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up:
      if(!editing) MenuItem::mainMenuPrev(_EditLCDBrightness_);
      else CONFIG->DisplayBrightness = constrain(CONFIG->DisplayBrightness+1, 1, 8);
      break;
    
    case Down:
      if(!editing) MenuItem::mainMenuNext(_EditLCDBrightness_);
      else CONFIG->DisplayBrightness = constrain(CONFIG->DisplayBrightness-1, 1, 8);
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditRPMBuffer == */
class EditRPMBufferMenuItem : public EditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_B,
    SEGMENT_u,
    SEGMENT_F,
    SEGMENT_F
  };
  
  uint8_t yesSegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_Y,
    SEGMENT_e,
    SEGMENT_S
  };
  
  uint8_t noSegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_BLANK,
    SEGMENT_N,
    SEGMENT_o
  };
  
  void onEnter(){
    EditingMenuItem::onEnter();
  }
  
  void onLeave(){
    if(didEdit) rpm->setAveragingDepth(CONFIG->RPMBuffer ? BUFFER_DEPTH : 0);
  }
  
  void onUpdate(){
    if(editing){
      display->setSegments(CONFIG->RPMBuffer ? yesSegments : noSegments);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up:
      if(!editing) MenuItem::mainMenuPrev(_EditRPMBuffer_);
      else CONFIG->RPMBuffer = !CONFIG->RPMBuffer;
      break;
    
    case Down:
      if(!editing) MenuItem::mainMenuNext(_EditRPMBuffer_);
      else CONFIG->RPMBuffer = !CONFIG->RPMBuffer;
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditRPMMeasureMode == */
class EditRPMMeasureModeMenuItem : public EditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_t,
    SEGMENT_Y,
    SEGMENT_P,
    SEGMENT_e
  };
  
  void onEnter(){
    EditingMenuItem::onEnter();
  }
  
  void onLeave(){
    if(didEdit){
      rpm->setMeasureMode(CONFIG->RPMMeasureMode);
      rpm->begin();
    }
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->RPMMeasureMode+1);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up:
      if(!editing) MenuItem::mainMenuPrev(_EditRPMMeasureMode_);
      else CONFIG->RPMMeasureMode = !CONFIG->RPMMeasureMode;
      break;
    
    case Down:
      if(!editing) MenuItem::mainMenuNext(_EditRPMMeasureMode_);
      else CONFIG->RPMMeasureMode = !CONFIG->RPMMeasureMode;
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditStationaryRPM == */
class EditStationaryRPMMenuItem : public RPMEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_S,
    SEGMENT_t,
    SEGMENT_A,
    SEGMENT_t
  };
  
  void onLeave(){
    if(didEdit) CONFIG->RPMStationary = constrain(CONFIG->RPMStationary, MIN_RPM_SETTING, PROFILE->RPMActivation);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->RPMStationary);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    CONFIG->RPMStationary = constrain(CONFIG->RPMStationary+difference, MIN_RPM_SETTING, PROFILE->RPMActivation);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::mainMenuPrev(_EditStationaryRPM_); break;
    case Down: if(!editing) MenuItem::mainMenuNext(_EditStationaryRPM_); break;
    
    case Left:
      MenuItem::enter(_ProfileListing_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditProfileLowRPM == */
class EditProfileLowRPMMenuItem : public RPMEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_BLANK,
    SEGMENT_L,
    SEGMENT_o
  };
  
  uint8_t offSegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_O,
    SEGMENT_F,
    SEGMENT_F
  };
  
  void onLeave(){
    if(didEdit) PROFILE->RPMLow = constrain(PROFILE->RPMLow, 0, PROFILE->RPMActivation);
  }
  
  void onUpdate(){
    if(editing){
      if(PROFILE->RPMLow > 0){
        display->showNumberDec(PROFILE->RPMLow);
      } else {
        display->setSegments(offSegments);
      }
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    //Low is special, since 0 == OFF, increases should start from stationary.
    if(PROFILE->RPMLow == 0){
      PROFILE->RPMLow = CONFIG->RPMStationary;
    }
    
    PROFILE->RPMLow = constrain(PROFILE->RPMLow+difference, CONFIG->RPMStationary, PROFILE->RPMActivation);
    
    //When we're back to stationary, that means we decreased into OFF.
    if(PROFILE->RPMLow == CONFIG->RPMStationary){
      PROFILE->RPMLow = 0;
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::profileMenuPrev(_EditProfileLowRPM_); break;
    case Down: if(!editing) MenuItem::profileMenuNext(_EditProfileLowRPM_); break;
    
    case Left:
      MenuItem::enter(_ProfileListing_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditProfileActivationRPM == */
class EditProfileActivationRPMMenuItem : public RPMEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_A,
    SEGMENT_c,
    SEGMENT_t,
    SEGMENT_i
  };
  
  void onLeave(){
    if(didEdit) PROFILE->RPMShift = max(PROFILE->RPMActivation, PROFILE->RPMShift);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(PROFILE->RPMActivation);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    PROFILE->RPMActivation = constrain(PROFILE->RPMActivation+difference, MIN_RPM_SETTING, PROFILE->RPMShift);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::profileMenuPrev(_EditProfileActivationRPM_); break;
    case Down: if(!editing) MenuItem::profileMenuNext(_EditProfileActivationRPM_); break;
    
    case Left:
      MenuItem::enter(_ProfileListing_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditProfileShiftRPM == */
class EditProfileShiftRPMMenuItem : public RPMEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_S,
    SEGMENT_h,
    SEGMENT_F,
    SEGMENT_t
  };
  
  void onLeave(){
    if(didEdit) PROFILE->RPMShift = constrain(PROFILE->RPMShift, PROFILE->RPMActivation, MAX_RPM_SETTING);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(PROFILE->RPMShift);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    PROFILE->RPMShift = constrain(PROFILE->RPMShift+difference, PROFILE->RPMActivation, MAX_RPM_SETTING);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::profileMenuPrev(_EditProfileShiftRPM_); break;
    case Down: if(!editing) MenuItem::profileMenuNext(_EditProfileShiftRPM_); break;
    
    case Left:
      MenuItem::enter(_ProfileListing_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditRPMStep == */
class EditRPMStepMenuItem : public EditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_S,
    SEGMENT_t, // SteP   (because a V is hard to write)
    SEGMENT_e,
    SEGMENT_P
  };
  
  uint8_t stepOptions[7] = { 1, 2, 5, 10, 20, 50, 100 };
  uint8_t stepIndex;
  
  void onEnter(){
    EditingMenuItem::onEnter();
    //Find which step we currently have.
    for(stepIndex=0; stepIndex<7; stepIndex++){
      if(stepOptions[stepIndex] >= CONFIG->RPMStep)
        break;
    }
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->RPMStep);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: 
      if(!editing) MenuItem::mainMenuPrev(_EditRPMStep_);
      else{
        stepIndex = min(stepIndex+1, 6);
        CONFIG->RPMStep = stepOptions[stepIndex];
      }
      break;
    
    case Down:
      if(!editing) MenuItem::mainMenuNext(_EditRPMStep_);
      else{
        stepIndex = max(stepIndex-1, 0);
        CONFIG->RPMStep = stepOptions[stepIndex];
      }
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditProfileRPMAnimation == */
class EditProfileRPMAnimationMenuItem : public EditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_A, // Ani   (because an M is hard to write)
    SEGMENT_n,
    SEGMENT_i
  };
  
  uint8_t animationNames[5][4] = {
    { SEGMENT_BLANK, SEGMENT_l, SEGMENT_t, SEGMENT_r },    // 0 = LeftToRight
    { SEGMENT_BLANK, SEGMENT_r, SEGMENT_t, SEGMENT_l },    // 1 = RightToLeft
    { SEGMENT_BLANK, SEGMENT_O, SEGMENT_u, SEGMENT_t },    // 2 = Outward
    { SEGMENT_BLANK, SEGMENT_BLANK, SEGMENT_I, SEGMENT_n },// 3 = Inward
    { SEGMENT_F, SEGMENT_U, SEGMENT_L, SEGMENT_L }         // 4 = Full bar
  };
  
  void onEnter(){
    EditingMenuItem::onEnter();
  }
  
  void onUpdate(){
    if(editing){
      display->setSegments(animationNames[PROFILE->RPMAnimation]);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up:
      if(!editing) MenuItem::profileMenuPrev(_EditProfileRPMAnimation_);
      else PROFILE->RPMAnimation = max(PROFILE->RPMAnimation-1, 0);
      break;
    
    case Down:
      if(!editing) MenuItem::profileMenuNext(_EditProfileRPMAnimation_);
      else PROFILE->RPMAnimation = min(PROFILE->RPMAnimation+1, 4);
      break;
    
    case Left:
      MenuItem::enter(_ProfileListing_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditProfileColors == */
class EditProfileColorsMenuItem : public MenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_L,
    SEGMENT_r,
    SEGMENT_S
  };
  
  void onEnter(){
    display->setSegments(displaySegments);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    switch (event) {
    
    case Up: MenuItem::profileMenuPrev(_EditProfileColors_); break;
    case Down: MenuItem::profileMenuNext(_EditProfileColors_); break;
    
    case Right:
      MenuItem::enter(FIRST_COLOR_MENU_ITEM);
      break;
    
    case Left:
      MenuItem::enter(_ProfileListing_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditColorLow == */
class EditColorLowMenuItem : public ColorEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_BLANK,
    SEGMENT_L,
    SEGMENT_o
  };
  
  void onEnter(){
    colorIndex = PROFILE->CLow;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      PROFILE->CLow = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorLow_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorLow_); break;
    case Left: MenuItem::enter(_EditProfileColors_); break;
  }}
  
};

/* ==  EditColorPart1 == */
class EditColorPart1MenuItem : public ColorEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_P,
    SEGMENT_t,
    SEGMENT_1
  };
  
  void onEnter(){
    colorIndex = PROFILE->CPart1;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      PROFILE->CPart1 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorPart1_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorPart1_); break;
    case Left: MenuItem::enter(_EditProfileColors_); break;
  }}
  
};

/* ==  EditColorPart2 == */
class EditColorPart2MenuItem : public ColorEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_P,
    SEGMENT_t,
    SEGMENT_2
  };
  
  void onEnter(){
    colorIndex = PROFILE->CPart2;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      PROFILE->CPart2 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorPart2_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorPart2_); break;
    case Left: MenuItem::enter(_EditProfileColors_); break;
  }}
  
};

/* ==  EditColorPart3 == */
class EditColorPart3MenuItem : public ColorEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_P,
    SEGMENT_t,
    SEGMENT_3
  };
  
  void onEnter(){
    colorIndex = PROFILE->CPart3;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      PROFILE->CPart3 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorPart3_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorPart3_); break;
    case Left: MenuItem::enter(_EditProfileColors_); break;
  }}
  
};

/* ==  EditColorFlash1 == */
class EditColorFlash1MenuItem : public ColorEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_F,
    SEGMENT_L,
    SEGMENT_1
  };
  
  void onEnter(){
    colorIndex = PROFILE->CFlash1;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      PROFILE->CFlash1 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorFlash1_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorFlash1_); break;
    case Left: MenuItem::enter(_EditProfileColors_); break;
  }}
  
};

/* ==  EditColorFlash2 == */
class EditColorFlash2MenuItem : public ColorEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_F,
    SEGMENT_L,
    SEGMENT_2
  };
  
  void onEnter(){
    colorIndex = PROFILE->CFlash2;
    allowBlack = true;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      PROFILE->CFlash2 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorFlash2_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorFlash2_); break;
    case Left: MenuItem::enter(_EditProfileColors_); break;
  }}
  
};

/* ==  Reset == */
class ResetMenuItem : public MenuItem {
  
  bool sure;
  
  uint8_t displaySegments[4] = {
    SEGMENT_r,
    SEGMENT_S,
    SEGMENT_e,
    SEGMENT_t
  };
  
  uint8_t sureSegments[4] = {
    SEGMENT_S,
    SEGMENT_u,
    SEGMENT_r,
    SEGMENT_e
  };
  
  void onEnter(){
    sure=false;
    display->setSegments(displaySegments);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    switch (event) {
    
    case Up: MenuItem::mainMenuPrev(_Reset_); break;
    case Down: MenuItem::mainMenuNext(_Reset_); break;
    
    case Right:
      if(!sure){
        sure=true;
        display->setSegments(sureSegments);
      }
      else{
        CONFIG->reset();
        rpm->setAveragingDepth(CONFIG->RPMBuffer ? BUFFER_DEPTH : 0);
        rpm->setPulsesPerRevolution(CONFIG->PPR);
        rpm->setMeasureMode(CONFIG->RPMMeasureMode);
        rpm->begin();
        animator->updateColors();
        MenuItem::enter(_Home_);
      }
      break;
    
    case Left:
      if(sure){
        sure = false;
        display->setSegments(displaySegments);
      }
      else{
        MenuItem::enter(_Home_);
        CONFIG->save();
      }
      break;
    
  }}
  
};

/* ==  DeleteProfile == */
class DeleteProfileMenuItem : public MenuItem {
  
  bool sure;
  
  uint8_t displaySegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_d,
    SEGMENT_e,
    SEGMENT_l
  };
  
  uint8_t sureSegments[4] = {
    SEGMENT_S,
    SEGMENT_u,
    SEGMENT_r,
    SEGMENT_e
  };
  
  void onEnter(){
    sure=false;
    display->setSegments(displaySegments);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    switch (event) {
    
    case Up: MenuItem::profileMenuPrev(_DeleteProfile_); break;
    case Down: MenuItem::profileMenuNext(_DeleteProfile_); break;
    
    case Right:
      if(!sure){
        sure=true;
        display->setSegments(sureSegments);
      }
      else{
        CONFIG->deleteProfile(PROFILE);
        animator->updateColors();
        MenuItem::enter(_ProfileListing_);
      }
      break;
    
    case Left:
      if(sure){
        sure = false;
        display->setSegments(displaySegments);
      }
      else{
        MenuItem::enter(_ProfileListing_);
        CONFIG->save();
      }
      break;
    
  }}
  
};

/* ==  EditTime == */
class EditTimeMenuItem : public MenuItem {
  
  enum editWhat : uint8_t
  {
    NOTHING = 0,
    YEAR = 1,
    MONTH = 2,
    DAY = 3,
    HOUR = 4,
    MINUTE = 5,
    SET = 6
  };
  
  uint8_t displaySegments[4] = {
    SEGMENT_BLANK,
    SEGMENT_BLANK,
    SEGMENT_t,
    SEGMENT_i
  };
  
  uint16_t editValue;
  uint8_t desiredOutput[4];
  uint8_t editState;
  uint8_t prevState;
  Time newTime = Time(0, 0, 0, 0, 0, 0, Time::kSunday);
  
  void onEnter(){
    desiredOutput[0] = displaySegments[0];
    desiredOutput[1] = displaySegments[1];
    desiredOutput[2] = displaySegments[2];
    desiredOutput[3] = displaySegments[3];
    editState = (uint8_t)NOTHING;
    prevState = (uint8_t)NOTHING;
    editValue = 0;
    newTime = rtcClock->time();
    newTime.sec = 0;
  }
  
  void onUpdate(){
    uint8_t output[4] = {
      desiredOutput[0],
      desiredOutput[1],
      desiredOutput[2],
      desiredOutput[3]
    };
    bool blink = millis()/ANIM_TURBO%8<3;
    switch(editState){
      case HOUR:
        if(blink){
          output[0] = SEGMENT_BLANK;
          output[1] = SEGMENT_BLANK;
        };
        display->setColon(true);
        break;
      case MINUTE:
        if(blink){
          output[2] = SEGMENT_BLANK;
          output[3] = SEGMENT_BLANK;
        };
        display->setColon(true);
        break;
      default:
        display->setColon(false);
        break;
    }
    display->setSegments(output);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    
    // Before moving state.
    switch(editState){
      
      // When we've done nothing yet.
      case NOTHING:
        switch (event) {
          case Up: MenuItem::mainMenuPrev(_EditTime_); break;
          case Down: MenuItem::mainMenuNext(_EditTime_); break;
          case Right: editState++; break;
          case Left:
            MenuItem::enter(_Home_);
            break;
        }
        break;
      
      // Most types support jumps by 10.
      case YEAR:
      case DAY:
      case HOUR:
      case MINUTE:
        switch(event){
          case HoldUp: editValue += 10; break;
          case HoldDown: editValue -= 10; break;
        }
      
      // For all the other values, just jump by 1. And support moving state.
      case MONTH:
        switch(event){
          case Up: editValue++; break;
          case Down: editValue--; break;
          case Right: editState++; break;
          case Left: editState--; break;
        }
        break;
      
    }
    
    // After moving state.
    switch(editState){
      case NOTHING:
        desiredOutput[0] = displaySegments[0];
        desiredOutput[1] = displaySegments[1];
        desiredOutput[2] = displaySegments[2];
        desiredOutput[3] = displaySegments[3];
        break;
      case YEAR:
        if(prevState != editState){
          editValue = newTime.yr;
          prevState = editState;
        }else{
          //DS1302 supports only 100 years, 2k defined by library as starting point.
          editValue = constrain(editValue, 2000, 2099);
          newTime.yr = editValue;
        }
        desiredOutput[0] = display->encodeDigit(editValue/1000%10);
        desiredOutput[1] = display->encodeDigit(editValue/100%10);
        desiredOutput[2] = display->encodeDigit(editValue/10%10);
        desiredOutput[3] = display->encodeDigit(editValue%10);
        break;
      case MONTH:
        if(prevState != editState){
          editValue = newTime.mon;
          prevState = editState;
        }else{
          if(editValue > 100) editValue = 1; //Watch for overflows.
          else editValue = constrain(editValue, 1, 12);
          newTime.mon = editValue;
        }
        desiredOutput[0] = SEGMENT_n;
        desiredOutput[1] = SEGMENT_BLANK;
        desiredOutput[2] = display->encodeDigit(editValue/10%10);
        desiredOutput[3] = display->encodeDigit(editValue%10);
        break;
      case DAY:
        if(prevState != editState){
          editValue = newTime.date;
          prevState = editState;
        }else{
          if(editValue > 100) editValue = 1; //Watch for overflows.
          else editValue = constrain(editValue, 1, 31);
          newTime.date = editValue;
        }
        desiredOutput[0] = SEGMENT_d;
        desiredOutput[1] = SEGMENT_BLANK;
        desiredOutput[2] = display->encodeDigit(editValue/10%10);
        desiredOutput[3] = display->encodeDigit(editValue%10);
        break;
      case HOUR:
        if(prevState != editState){
          editValue = newTime.hr;
          prevState = editState;
        }else{
          if(editValue > 100) editValue = 0; //Watch for overflows.
          else editValue = constrain(editValue, 0, 23);
          newTime.hr = editValue;
        }
        desiredOutput[0] = display->encodeDigit(editValue/10%10);
        desiredOutput[1] = display->encodeDigit(editValue%10);
        desiredOutput[2] = display->encodeDigit(newTime.min/10%10);
        desiredOutput[3] = display->encodeDigit(newTime.min%10);
        break;
      case MINUTE:
        if(prevState != editState){
          editValue = newTime.min;
          prevState = editState;
        }else{
          if(editValue > 100) editValue = 0; //Watch for overflows.
          else editValue = constrain(editValue, 0, 59);
          newTime.min = editValue;
        }
        desiredOutput[0] = display->encodeDigit(newTime.hr/10%10);
        desiredOutput[1] = display->encodeDigit(newTime.hr%10);
        desiredOutput[2] = display->encodeDigit(editValue/10%10);
        desiredOutput[3] = display->encodeDigit(editValue%10);
        break;
      
      // We are apparently done, save our things and return to the NOTHING state.
      case SET:
        rtcClock->writeProtect(false);
        rtcClock->time(newTime);
        rtcClock->writeProtect(true);
        editState=0;
        desiredOutput[0] = displaySegments[0];
        desiredOutput[1] = displaySegments[1];
        desiredOutput[2] = displaySegments[2];
        desiredOutput[3] = displaySegments[3];
        animator->showBlockingRunlight(CRGB::Green);
        break;
    }
    
  }
  
};

/* ==  EditColorSegments == */
class EditColorSegmentsMenuItem : public MenuItem {
  
  enum editWhat : uint8_t
  {
    NOTHING = 0,
    FULL_SEGMENT1 = 1,
    FULL_SEGMENT2 = 2,
    HALF_SEGMENT1 = 3,
    HALF_SEGMENT2 = 4,
    SET = 5
  };
  
  uint8_t displaySegments[4] = {
    SEGMENT_C,
    SEGMENT_S,
    SEGMENT_e,
    SEGMENT_g
  };
  
  uint16_t editValue;
  uint8_t editState;
  uint8_t prevState;
  uint8_t segments[4];
  
  void onEnter(){
    segments[0] = CONFIG->FullSegment1;
    segments[1] = CONFIG->FullSegment2;
    segments[2] = CONFIG->HalfSegment1;
    segments[3] = CONFIG->HalfSegment2;
    editState = (uint8_t)NOTHING;
    prevState = (uint8_t)NOTHING;
    editValue = 0;
    display->setSegments(displaySegments);
  }
  
  void onUpdate(){
    bool blink = millis()/ANIM_TURBO%8<3;
    switch(editState){
      case FULL_SEGMENT1:
        animator->showSegmentPreview(segments[0],segments[1],(blink?1:0),false);
        break;
      case FULL_SEGMENT2:
        animator->showSegmentPreview(segments[0],segments[1],(blink?2:0),false);
        break;
      case HALF_SEGMENT1:
        animator->showSegmentPreview(segments[2],segments[3],(blink?1:0),true);
        break;
      case HALF_SEGMENT2:
        animator->showSegmentPreview(segments[2],segments[3],(blink?2:0),true);
        break;
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    
    // Before moving state.
    switch(editState){
      
      // When we've done nothing yet.
      case NOTHING:
        switch (event) {
          case Up: MenuItem::mainMenuPrev(_EditColorSegments_); break;
          case Down: MenuItem::mainMenuNext(_EditColorSegments_); break;
          case Right: editState++; break;
          case Left:
            MenuItem::enter(_Home_);
            break;
        }
        break;
      
      // Just adjust by 1. And support moving state.
      default:
        switch(event){
          case Up: editValue++; break;
          case Down: editValue--; break;
          case Right: editState++; break;
          case Left: editState--; break;
        }
        break;
      
    }
    
    int halfMax = ceil(NUMPIXELS/2.0);
    
    // After moving state.
    switch(editState){
      case NOTHING:
        animator->setFill(CRGB::Black);
        animator->show();
        break;
      case FULL_SEGMENT1:
        if(prevState != editState){
          editValue = segments[0];
          prevState = editState;
        }else{
          editValue = constrain(editValue, 1, NUMPIXELS-2);
          segments[0] = editValue;
          segments[1] = constrain(segments[1], 1, NUMPIXELS-editValue-1);
        }
        break;
      case FULL_SEGMENT2:
        if(prevState != editState){
          editValue = segments[1];
          prevState = editState;
        }else{
          editValue = constrain(editValue, 1, NUMPIXELS-segments[0]-1);
          segments[1] = editValue;
        }
        break;
      case HALF_SEGMENT1:
        if(prevState != editState){
          editValue = segments[2];
          prevState = editState;
        }else{
          editValue = constrain(editValue, 1, halfMax-2);
          segments[2] = editValue;
          segments[3] = constrain(segments[3], 1, halfMax-editValue-1);
        }
        break;
      case HALF_SEGMENT2:
        if(prevState != editState){
          editValue = segments[3];
          prevState = editState;
        }else{
          editValue = constrain(editValue, 1, halfMax-segments[2]-1);
          segments[3] = editValue;
        }
        break;
      
      // We are apparently done, save our things and return to the NOTHING state.
      case SET:
        CONFIG->FullSegment1 = segments[0];
        CONFIG->FullSegment2 = segments[1];
        CONFIG->HalfSegment1 = segments[2];
        CONFIG->HalfSegment2 = segments[3];
        CONFIG->save();
        animator->updateColors();
        editState=0;
        animator->showBlockingRunlight(CRGB::Green);
        break;
    }
    
  }
  
};

#endif
