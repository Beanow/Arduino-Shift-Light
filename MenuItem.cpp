
#include "MenuItem.h"
#include "_menu.h"

static MenuItem* getMenuItem(uint8_t itemIndex){switch(itemIndex){
  case _Home_: return new HomeMenuItem();
  case _QuickBrightness_: return new QuickBrightnessMenuItem();
  case _ProfileSwitcher_: return new ProfileSwitcherMenuItem();
  case _MainMenu_: return new MainMenuMenuItem();
  case _ProfileListing_: return new ProfileListingMenuItem();
  case _EditRPMBuffer_: return new EditRPMBufferMenuItem();
  case _EditRPMMeasureMode_: return new EditRPMMeasureModeMenuItem();
  case _EditPPR_: return new EditPPRMenuItem();
  case _EditStationaryRPM_: return new EditStationaryRPMMenuItem();
  case _EditRPMStep_: return new EditRPMStepMenuItem();
  case _EditTime_: return new EditTimeMenuItem();
  case _EditColorSegments_: return new EditColorSegmentsMenuItem();
  case _EditLCDBrightness_: return new EditLCDBrightnessMenuItem();
  case _EditProfileLowRPM_: return new EditProfileLowRPMMenuItem();
  case _EditProfileActivationRPM_: return new EditProfileActivationRPMMenuItem();
  case _EditProfileShiftRPM_: return new EditProfileShiftRPMMenuItem();
  case _EditProfileRPMAnimation_: return new EditProfileRPMAnimationMenuItem();
  case _EditProfileColors_: return new EditProfileColorsMenuItem();
  case _EditColorLow_: return new EditColorLowMenuItem();
  case _EditColorPart1_: return new EditColorPart1MenuItem();
  case _EditColorPart2_: return new EditColorPart2MenuItem();
  case _EditColorPart3_: return new EditColorPart3MenuItem();
  case _EditColorFlash1_: return new EditColorFlash1MenuItem();
  case _EditColorFlash2_: return new EditColorFlash2MenuItem();
  case _Reset_: return new ResetMenuItem();
  case _DeleteProfile_: return new DeleteProfileMenuItem();
}}

Config* MenuItem::CONFIG;
Profile* MenuItem::PROFILE;
RPMMeasure* MenuItem::rpm;
ButtonSet* MenuItem::buttons;
TM1637Display* MenuItem::display;
PixelAnimator* MenuItem::animator;
MenuItem* MenuItem::activeMenuItem;
DS1302* MenuItem::rtcClock;
uint8_t MenuItem::prevMenuItemIndex = _Home_;

void MenuItem::mainMenuNext(uint8_t currentIndex){
  currentIndex++;
  if(currentIndex > LAST_MAIN_MENU_ITEM){
    currentIndex = _MainMenu_;
  }
  enter(currentIndex);
}

void MenuItem::mainMenuPrev(uint8_t currentIndex){
  currentIndex--;
  if(currentIndex < _MainMenu_){
    currentIndex = LAST_MAIN_MENU_ITEM;
  }
  enter(currentIndex);
}

void MenuItem::colorMenuNext(uint8_t currentIndex){
  currentIndex++;
  if(currentIndex > LAST_COLOR_MENU_ITEM){
    currentIndex = FIRST_COLOR_MENU_ITEM;
  }
  enter(currentIndex);
}

void MenuItem::colorMenuPrev(uint8_t currentIndex){
  currentIndex--;
  if(currentIndex < FIRST_COLOR_MENU_ITEM){
    currentIndex = LAST_COLOR_MENU_ITEM;
  }
  enter(currentIndex);
}

void MenuItem::profileMenuNext(uint8_t currentIndex){
  currentIndex++;
  if(currentIndex > LAST_PROFILE_MENU_ITEM){
    currentIndex = FIRST_PROFILE_MENU_ITEM;
  }
  enter(currentIndex);
}

void MenuItem::profileMenuPrev(uint8_t currentIndex){
  currentIndex--;
  if(currentIndex < FIRST_PROFILE_MENU_ITEM){
    currentIndex = LAST_PROFILE_MENU_ITEM;
  }
  enter(currentIndex);
}

void MenuItem::enter(uint8_t itemIndex){
  enter(getMenuItem(itemIndex));
  prevMenuItemIndex = itemIndex;
}

void MenuItem::update(){
  if(activeMenuItem){
    activeMenuItem->onUpdate();
  }
}

void MenuItem::onButtonSetEvent(buttonSetEvent_t event){
  if(activeMenuItem){
    activeMenuItem->onButtonEvent(event);
  }
}

void MenuItem::setConfig(Config* c){
  CONFIG = c;
  PROFILE = &(CONFIG->Profiles[0]);
}

void MenuItem::setButtonSet(ButtonSet* b){
  buttons = b;
  buttons->setEventHandler(MenuItem::onButtonSetEvent);
}

void MenuItem::setAnimator(PixelAnimator* a){
  animator = a;
}

void MenuItem::setRTCClock(DS1302* c){
  rtcClock = c;
}

void MenuItem::setRPMMeasure(RPMMeasure* r){
  rpm = r;
}

void MenuItem::setDisplay(TM1637Display* d){
  display = d;
}

void MenuItem::enter(MenuItem* item){
  if(activeMenuItem){
    activeMenuItem->onLeave();
    delete activeMenuItem;
  }
  display->setColon(false);
  animator->setFill(CRGB::Black);
  activeMenuItem = item;
  activeMenuItem->onEnter();
}
