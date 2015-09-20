#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <TM1637Display.h>
#include <FreqMeasure.h>
#include <InterruptFreqMeasure.h>
#include "Config.h"
#include "RPMMeasure.h"
#include "ButtonSet.h"
#include "MenuItem.h"
#include "PixelAnimator.h"
#include "_menu.h"
#include "_defines.h"

//Initialize required objects.
TM1637Display* display = new TM1637Display(LCDCLK, LCDDIO);
ButtonSet* buttons = new ButtonSet(BTN_LEFT, BTN_UP, BTN_RIGHT, BTN_DOWN);
RPMMeasure* rpm = new RPMMeasure(RPM_INT_PIN, RPM_INT_NUM, RPM_TI1_PIN);
Config* CONFIG = new Config();
PixelAnimator* animator = new PixelAnimator(CONFIG);

void setup(){
  
  // Let the debugging begin.
  // Serial.begin(9600);
  
  //Load our config from defaults/EEPROM.
  CONFIG->load();
  
  //Initialize pixel animator.
  animator->setup();
  
  //Bind the required objects to the MenuItems.
  MenuItem::setConfig(CONFIG);
  MenuItem::setRPMMeasure(rpm);
  MenuItem::setDisplay(display);
  MenuItem::setAnimator(animator);
  MenuItem::setButtonSet(buttons);
  
  //Go to the home page.
  MenuItem::enter(new HomeMenuItem());
  
  //Initialize RPM sensor.
  rpm->setAveragingDepth(CONFIG->RPMBuffer ? BUFFER_DEPTH : 0);
  rpm->setPulsesPerRevolution(CONFIG->PPR);
  rpm->setMeasureMode(CONFIG->RPMMeasureMode);
  rpm->begin();
  
  //Start tracking buttons.
  buttons->begin();
  
}

void loop() {
  
  //Update all the things!
  buttons->update();
  rpm->update();
  MenuItem::update();
  
  delay(UPDATE_INTERVAL);
  
}
