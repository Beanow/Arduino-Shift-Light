/**
 * A class that tracks the 4-way button states.
 */

#include <Arduino.h>
#include <Button.h>
#include "ButtonSet.h"

ButtonSet::ButtonSet(uint8_t pLeft, uint8_t pUp, uint8_t pRight, uint8_t pDown){
  pins[0] = pLeft;
  pins[1] = pUp;
  pins[2] = pRight;
  pins[3] = pDown;
}

ButtonSet::~ButtonSet(){
  if(hasSetup){
    for (int i = 0; i < NUM_BUTTONS; i++){
      pinMode(pins[i], INPUT);
      delete buttons[i];
    }
  }
}

void ButtonSet::begin(){
  //Start tracking the pins.
  for (int i = 0; i < NUM_BUTTONS; i++){
    buttons[i] = new Button(pins[i]);
    buttons[i]->setHoldThreshold(10000);
  }
  longReleaseQueue=0;
  hasSetup = true;
}

void ButtonSet::setEventHandler(ButtonSetEventHandler callback){
  cb_event = callback;
}

void ButtonSet::update(){
  if(!hasSetup) return;
  
  //Placeholder for the final event.
  uint8_t event = 0;
  
  //Loop all the buttons, even if we find multiple events.
  for (int i = 0; i < NUM_BUTTONS; i++){
    
    //Shortcut for this button.
    Button* btn = buttons[i];
    
    //See if the button was released to constitute a "click".
    //Released means: last read it was pressed, but now it is not.
    //Note: this does an internal state update of the Button object.
    if((!btn->isPressed()) && btn->stateChanged()){
      //Make sure the event should not be ignored as a long press release.
      if(bitRead(longReleaseQueue,i)){
        bitWrite(longReleaseQueue,i,false);
        longReleaseQueue &= ~(1<<i);
        longPressCount[i] = 0;
      }else{
        event = (1<<i);
      }
    }
    
    //See if the button was long pressed yet.
    else if(btn->held(LONG_PRESS)){
      event = (1<<(i+NUM_BUTTONS));
      
      //Mark this button as long pressed, so the release is not considered a click.
      bitWrite(longReleaseQueue,i,true);
      longPressCount[i]++;
    }
    
    //See if we should repeat the long press.
    else if(bitRead(longReleaseQueue,i) && btn->holdTime() > (LONG_PRESS+longPressCount[i]*LONG_PRESS_REPEAT)){
      event = (1<<(i+NUM_BUTTONS));
      bitWrite(longReleaseQueue,i,true);
      longPressCount[i]++;
    }
    
  }
  
  //Now that all button states have been processed,
  //send whichever even has not been overwritten.
  if(event > 0 && cb_event){
    cb_event(event);
  }
}
