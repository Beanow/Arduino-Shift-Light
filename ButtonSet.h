/**
 * A class that tracks the 4-way button states.
 */

#ifndef ButtonSet_h
#define ButtonSet_h

#include <Button.h>

//Define the button count.
#define NUM_BUTTONS 4

//Configure button timings.
#define LONG_PRESS 1000
#define LONG_PRESS_REPEAT 750

typedef uint8_t buttonSetEvent_t;
class ButtonSetEvent {
  public:
    enum buttonMask: buttonSetEvent_t{
      Left     = 0b00000001,
      Up       = 0b00000010,
      Right    = 0b00000100,
      Down     = 0b00001000,
      HoldLeft = 0b00010000,
      HoldUp   = 0b00100000,
      HoldRight= 0b01000000,
      HoldDown = 0b10000000,
    };
};

typedef void (*ButtonSetEventHandler)(buttonSetEvent_t);

class ButtonSet {
  
  public:
    ButtonSet(uint8_t pinLeft, uint8_t pinUp, uint8_t pinRight, uint8_t pinDown);
    ~ButtonSet();
    
    void
      begin(),
      setEventHandler(ButtonSetEventHandler callback),
      update(void);
  
  private:
    
    bool
      hasSetup;
    
    uint8_t
      longReleaseQueue,
      longPressCount[NUM_BUTTONS],
      pins[NUM_BUTTONS];
    
    Button*
      buttons[NUM_BUTTONS];
    
    ButtonSetEventHandler
      cb_event;
  
};

#endif
