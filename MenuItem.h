/**
 * A class that handles menu navigation and actions.
 */

#ifndef MenuItem_h
#define MenuItem_h

#include <Arduino.h>
#include <DS1302.h>
#include "PixelAnimator.h"
#include "RPMMeasure.h"
#include "ButtonSet.h"
#include "Config.h"
#include <TM1637Display.h>
#include "_menuHelpers.h"

class MenuItem {
  
  public:
    static void update(void);
    static void onButtonSetEvent(buttonSetEvent_t event);
    static void setConfig(Config* config);
    static void setRPMMeasure(RPMMeasure* rpm);
    static void setButtonSet(ButtonSet* buttons);
    static void setDisplay(TM1637Display* display);
    static void setAnimator(PixelAnimator* animator);
    static void setRTCClock(DS1302* rtcClock);
    static void enter(MenuItem* item);
    static void enter(uint8_t itemIndex);
    static void mainMenuNext(uint8_t currentIndex);
    static void mainMenuPrev(uint8_t currentIndex);
    static void colorMenuNext(uint8_t currentIndex);
    static void colorMenuPrev(uint8_t currentIndex);
    static void profileMenuNext(uint8_t currentIndex);
    static void profileMenuPrev(uint8_t currentIndex);
    virtual void onEnter(){}
    virtual void onButtonEvent(buttonSetEvent_t event){}
    virtual void onUpdate(){}
    virtual void onLeave(){}
  
  protected:
    static Config* CONFIG;
    static Profile* PROFILE;
    static RPMMeasure* rpm;
    static ButtonSet* buttons;
    static TM1637Display* display;
    static PixelAnimator* animator;
    static MenuItem* activeMenuItem;
    static DS1302* rtcClock;
    static uint8_t prevMenuItemIndex;
  
};

class EditingMenuItem : public MenuItem {
  
  public:
    
    virtual void onEnter(){
      editing = false;
      didEdit = false;
    }
    
    virtual void onButtonEvent(buttonSetEvent_t event){
      if(event == Right){
        editing = !editing;
        if(editing){ didEdit = true; }
      }
    }
  
  protected:
    bool editing;
    bool didEdit;
  
};

class RPMEditingMenuItem : public EditingMenuItem {
  
  public:
    virtual void onButtonEvent(buttonSetEvent_t event){
      EditingMenuItem::onButtonEvent(event);
      if(!editing) return;
      switch(event){
        case Up: onValueChange(100); break;
        case Down: onValueChange(-100); break;
        case HoldUp: onValueChange(1000); break;
        case HoldDown: onValueChange(-1000); break;
      }
    }
  
  protected:
    virtual void onValueChange(int difference);
  
};

class ColorEditingMenuItem : public EditingMenuItem {
  
  public:
    virtual void onEnter(){
      EditingMenuItem::onEnter();
      color = ColorPicker::at(colorIndex, allowBlack);
    }
    virtual void onButtonEvent(buttonSetEvent_t event){
      EditingMenuItem::onButtonEvent(event);
      if(!editing) return;
      switch(event){
        case Up: color = ColorPicker::prev(colorIndex, allowBlack); break;
        case Down: color = ColorPicker::next(colorIndex, allowBlack); break;
        case Left: animator->setFill(CRGB::Black); animator->show(); break;
      }
    }
    void onUpdate(){
      animator->setFill(editing ? color : CRGB::Black);
      animator->setEdges(editing && millis() / ANIM_FAST % 2 ? CRGB::White : CRGB::Black);
      animator->show();
    }
    
  protected:
    uint8_t colorIndex;
    CRGB color;
    bool allowBlack;
  
};

#endif
