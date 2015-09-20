
#include "RPMMeasure.h"

static bool showOne = true;

inline uint32_t readInterruptInterval(){
  return InterruptFreqMeasure.read();
};

inline uint32_t readTimerInterval(){
  return FreqMeasure.read();
};

RPMMeasure::RPMMeasure(uint8_t _interruptPin, uint8_t _interruptNum, uint8_t _timer1Pin) {
  setPulsesPerRevolution(DEFAULT_PPR);
  setAveragingDepth(DEFAULT_AVERAGING_DEPTH);
  measureMode=RPMMeasureMode::None;
  interruptPin = _interruptPin;
  interruptNum = _interruptNum;
  timer1Pin = _timer1Pin;
  begun=false;
}

RPMMeasure::~RPMMeasure() {
  end();
  if(rpmBuffer){
    delete [] rpmBuffer;
    rpmBuffer = NULL;
  }
}

void RPMMeasure::setAveragingDepth(uint8_t depth) {
  if(rpmBuffer){
    delete [] rpmBuffer;
    rpmBuffer = NULL;
  }
  
  averagingDepth = depth+1;
  bufferHead = 0;
  if(averagingDepth > 1){
    rpmBuffer = new uint16_t[averagingDepth];
  }
  
}

void RPMMeasure::setPulsesPerRevolution(uint8_t ppr) {
  pulsesPerRevolution = ppr;
}

void RPMMeasure::setMeasureMode(measureMode_t mode){
  end();
  measureMode=mode;
}

void RPMMeasure::update(){
  if(measureMode == RPMMeasureMode::None || !begun) return;
  
  uint8_t available;
  IntervalReader readInterval;
  bool needsUpdate;
  
  //Depending on our mode we use different classes.
  switch (measureMode){
    
    case RPMMeasureMode::Interrupt:
      available = InterruptFreqMeasure.available();
      readInterval = readInterruptInterval;
      break;
    
    case RPMMeasureMode::Timer1:
      available = FreqMeasure.available();
      readInterval = readTimerInterval;
      break;
    
  }
  
  //Store our time.
  uint32_t now = millis();
  
  //If we don't have any values to read, there's only the question whether we should insert a dead RPM (0).
  if(available == 0){
    
    //If the DEAD_RPM_TIME value was exceeded, insert a 0 and reset the timer.
    if(lastReading + DEAD_RPM_TIME > now){
      lastReading=now;
      rpmBuffer[bufferHead] = 0;
      bufferHead++;
      
      //Wrap back around if we need to.
      if(bufferHead >= averagingDepth){
        bufferHead = 0;
      }
      
      rpm=0;
    }
    
    //Return either way, because there's nothing to read.
    return;
    
  }
  
  //Since we do have readings available, bump the lastReading value.
  lastReading=now;
  
  //If we have more readings available than we want to use, first flush excess readings.
  if(available > averagingDepth){
    int flushCount = available-averagingDepth;
    for(int i = 0; i < flushCount; i++){
      readInterval();
    }
    available -= flushCount;
  }
  
  //See if we need to average the RPM or not.
  if(averagingDepth > 1){
    
    //Insert the (remaining) new values into the buffer.
    while(available > 0){
      rpmBuffer[bufferHead] = intervalToRPM(readInterval());
      bufferHead++;
      
      //Wrap back around if we need to.
      if(bufferHead >= averagingDepth){
        bufferHead = 0;
      }
      
      available--;
    }
    
    showOne = true;
    
    //Perform the averaging to store our updated RPM value.
    uint32_t rpmSum = 0;
    
    //Rewind the head through our averaging depth.
    long avgStart = (long)bufferHead - averagingDepth;
    
    //Loop it around to the start if need be.
    if(avgStart < 0){
      avgStart = averagingDepth+avgStart;
    }
    
    //Now convert to 8 bits, and iterate until we reach the head.
    uint8_t avgHead = avgStart;
    uint8_t readCount = averagingDepth;
    while(readCount > 0){
      rpmSum += rpmBuffer[avgHead];
      readCount--;
      avgHead++;
      if(avgHead >= averagingDepth){
        avgHead=0;
      }
    }
    
    //Store the average.
    rpm = rpmSum / averagingDepth;
    
  }
  
  //Otherwise simply store the latest value.
  else{
    rpm = intervalToRPM(readInterval());
  }
  
}

void RPMMeasure::begin(){
  switch (measureMode){
    case RPMMeasureMode::None: return;
    
    case RPMMeasureMode::Interrupt:
      InterruptFreqMeasure.begin(interruptPin, interruptNum);
      break;
    
    case RPMMeasureMode::Timer1:
      pinMode(timer1Pin, INPUT);
      FreqMeasure.setOverflowIgnoreTicks(FREQMEASURE_OVERFLOW_IGNORE_TICKS);
      FreqMeasure.begin();
      break;
    
  }
  lastReading=millis();
  begun = true;
}

void RPMMeasure::end(){
  switch (measureMode){
    case RPMMeasureMode::Interrupt:
      InterruptFreqMeasure.end();
      break;
    case RPMMeasureMode::Timer1:
      FreqMeasure.end();
      break;
  }
  measureMode=RPMMeasureMode::None;
  rpm=0;
  begun = false;
}

uint16_t RPMMeasure::getRPM(){
  return rpm;
}

uint16_t RPMMeasure::intervalToRPM(uint32_t interval){
  switch(measureMode){
    case RPMMeasureMode::Interrupt:
      return 60e6/interval/pulsesPerRevolution;
    case RPMMeasureMode::Timer1:
      float clockSpeed;
      #if defined(__AVR__)
        clockSpeed = F_CPU;
      #elif defined(__arm__) && defined(TEENSYDUINO)
        clockSpeed = F_BUS;
      #endif
      return clockSpeed*60/interval/pulsesPerRevolution;
  }
  
  return 0;
}
