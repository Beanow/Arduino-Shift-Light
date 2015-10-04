
#ifndef _menu_h
#define _menu_h

#include "MenuItem.h"
#include <FastLED.h>
// #include <DS1302.h>
#include "_defines.h"
#include "_menuHelpers.h"

//These need to stay at the top, and you can't use 0.
//Their numbers also define the menu ordering.
#define _Home_ 1
#define _QuickBrightness_ 2

#define _MainMenu_ 10 //This is always the first main menu item.
#define _EditStationaryRPM_ 11
#define _EditLowRPM_ 12
#define _EditActivationRPM_ 13
#define _EditShiftRPM_ 14
#define _EditRPMAnimation_ 15
#define _EditColors_ 16
#define _EditLCDBrightness_ 17
#define _EditRPMStep_ 18
#define _EditTime_ 19
#define _EditPPR_ 20
#define _EditRPMBuffer_ 21
#define _EditRPMMeasureMode_ 22
#define _Reset_ 23
#define LAST_MAIN_MENU_ITEM _Reset_ //Constraint the main menu.

#define _EditColorLow_ 30
#define _EditColorPart1_ 31
#define _EditColorPart2_ 32
#define _EditColorPart3_ 33
#define _EditColorFlash1_ 34
#define _EditColorFlash2_ 35
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
    if(didEdit) CONFIG->RPMStationary = constrain(CONFIG->RPMStationary, MIN_RPM_SETTING, CONFIG->CurrentProfile->RPMActivation);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->RPMStationary);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    CONFIG->RPMStationary = constrain(CONFIG->RPMStationary+difference, MIN_RPM_SETTING, CONFIG->CurrentProfile->RPMActivation);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::mainMenuPrev(_EditStationaryRPM_); break;
    case Down: if(!editing) MenuItem::mainMenuNext(_EditStationaryRPM_); break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditLowRPM == */
class EditLowRPMMenuItem : public RPMEditingMenuItem {
  
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
    if(didEdit) CONFIG->CurrentProfile->RPMLow = constrain(CONFIG->CurrentProfile->RPMLow, 0, CONFIG->CurrentProfile->RPMActivation);
  }
  
  void onUpdate(){
    if(editing){
      if(CONFIG->CurrentProfile->RPMLow > 0){
        display->showNumberDec(CONFIG->CurrentProfile->RPMLow);
      } else {
        display->setSegments(offSegments);
      }
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    //Low is special, since 0 == OFF, increases should start from stationary.
    if(CONFIG->CurrentProfile->RPMLow == 0){
      CONFIG->CurrentProfile->RPMLow = CONFIG->RPMStationary;
    }
    
    CONFIG->CurrentProfile->RPMLow = constrain(CONFIG->CurrentProfile->RPMLow+difference, CONFIG->RPMStationary, CONFIG->CurrentProfile->RPMActivation);
    
    //When we're back to stationary, that means we decreased into OFF.
    if(CONFIG->CurrentProfile->RPMLow == CONFIG->RPMStationary){
      CONFIG->CurrentProfile->RPMLow = 0;
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::mainMenuPrev(_EditLowRPM_); break;
    case Down: if(!editing) MenuItem::mainMenuNext(_EditLowRPM_); break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditActivationRPM == */
class EditActivationRPMMenuItem : public RPMEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_A,
    SEGMENT_c,
    SEGMENT_t,
    SEGMENT_i
  };
  
  void onLeave(){
    if(didEdit) CONFIG->CurrentProfile->RPMShift = max(CONFIG->CurrentProfile->RPMActivation, CONFIG->CurrentProfile->RPMShift);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->CurrentProfile->RPMActivation);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    CONFIG->CurrentProfile->RPMActivation = constrain(CONFIG->CurrentProfile->RPMActivation+difference, MIN_RPM_SETTING, CONFIG->CurrentProfile->RPMShift);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::mainMenuPrev(_EditActivationRPM_); break;
    case Down: if(!editing) MenuItem::mainMenuNext(_EditActivationRPM_); break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditShiftRPM == */
class EditShiftRPMMenuItem : public RPMEditingMenuItem {
  
  uint8_t displaySegments[4] = {
    SEGMENT_S,
    SEGMENT_h,
    SEGMENT_F,
    SEGMENT_t
  };
  
  void onLeave(){
    if(didEdit) CONFIG->CurrentProfile->RPMShift = constrain(CONFIG->CurrentProfile->RPMShift, CONFIG->CurrentProfile->RPMActivation, MAX_RPM_SETTING);
  }
  
  void onUpdate(){
    if(editing){
      display->showNumberDec(CONFIG->CurrentProfile->RPMShift);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onValueChange(int difference){
    CONFIG->CurrentProfile->RPMShift = constrain(CONFIG->CurrentProfile->RPMShift+difference, CONFIG->CurrentProfile->RPMActivation, MAX_RPM_SETTING);
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    RPMEditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up: if(!editing) MenuItem::mainMenuPrev(_EditShiftRPM_); break;
    case Down: if(!editing) MenuItem::mainMenuNext(_EditShiftRPM_); break;
    
    case Left:
      MenuItem::enter(_Home_);
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

/* ==  EditRPMAnimation == */
class EditRPMAnimationMenuItem : public EditingMenuItem {
  
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
      display->setSegments(animationNames[CONFIG->CurrentProfile->RPMAnimation]);
    } else {
      display->setSegments(displaySegments);
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    EditingMenuItem::onButtonEvent(event);
    switch (event) {
    
    case Up:
      if(!editing) MenuItem::mainMenuPrev(_EditRPMAnimation_);
      else CONFIG->CurrentProfile->RPMAnimation = max(CONFIG->CurrentProfile->RPMAnimation-1, 0);
      break;
    
    case Down:
      if(!editing) MenuItem::mainMenuNext(_EditRPMAnimation_);
      else CONFIG->CurrentProfile->RPMAnimation = min(CONFIG->CurrentProfile->RPMAnimation+1, 4);
      break;
    
    case Left:
      MenuItem::enter(_Home_);
      CONFIG->save();
      break;
    
  }}
  
};

/* ==  EditColors == */
class EditColorsMenuItem : public MenuItem {
  
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
    
    case Up: MenuItem::mainMenuPrev(_EditColors_); break;
    case Down: MenuItem::mainMenuNext(_EditColors_); break;
    
    case Right:
      MenuItem::enter(_EditColorLow_);
      break;
    
    case Left:
      MenuItem::enter(_Home_);
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
    colorIndex = CONFIG->CurrentProfile->CLow;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      CONFIG->CurrentProfile->CLow = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorLow_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorLow_); break;
    case Left: MenuItem::enter(_EditColors_); break;
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
    colorIndex = CONFIG->CurrentProfile->CPart1;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      CONFIG->CurrentProfile->CPart1 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorPart1_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorPart1_); break;
    case Left: MenuItem::enter(_EditColors_); break;
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
    colorIndex = CONFIG->CurrentProfile->CPart2;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      CONFIG->CurrentProfile->CPart2 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorPart2_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorPart2_); break;
    case Left: MenuItem::enter(_EditColors_); break;
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
    colorIndex = CONFIG->CurrentProfile->CPart3;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      CONFIG->CurrentProfile->CPart3 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorPart3_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorPart3_); break;
    case Left: MenuItem::enter(_EditColors_); break;
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
    colorIndex = CONFIG->CurrentProfile->CFlash1;
    allowBlack = false;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      CONFIG->CurrentProfile->CFlash1 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorFlash1_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorFlash1_); break;
    case Left: MenuItem::enter(_EditColors_); break;
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
    colorIndex = CONFIG->CurrentProfile->CFlash2;
    allowBlack = true;
    ColorEditingMenuItem::onEnter();
    display->setSegments(displaySegments);
  }
  
  void onLeave(){
    if(didEdit){
      CONFIG->CurrentProfile->CFlash2 = colorIndex;
      animator->updateColors();
    }
  }
  
  void onButtonEvent(buttonSetEvent_t event){
    ColorEditingMenuItem::onButtonEvent(event);
    switch (event) {
    case Up: if(!editing) MenuItem::colorMenuPrev(_EditColorFlash2_); break;
    case Down: if(!editing) MenuItem::colorMenuNext(_EditColorFlash2_); break;
    case Left: MenuItem::enter(_EditColors_); break;
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
      MenuItem::enter(_Home_);
      CONFIG->save();
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

#endif
