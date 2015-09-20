
#ifndef RPMMeasure_h
#define RPMMeasure_h

#include <Arduino.h>
#include <FreqMeasure.h>
#include <InterruptFreqMeasure.h>

//Assume some arbitrary defaults.
#define DEFAULT_PPR 4
#define DEFAULT_AVERAGING_DEPTH 0

//At 0 RPM there would be no pulses,
//this value determines how long we will wait for a pulse before considering it 0 RPM.
//IE: 100RPM * 2PPR = 300ms
#define DEAD_RPM_TIME 300

//This is a calibration of FreqMeasure to limit false readings.
#define FREQMEASURE_OVERFLOW_IGNORE_TICKS 0xC000

//The maximum buffer depth for averaging.
#define MAX_RPM_BUF 12

typedef int8_t measureMode_t;
class RPMMeasureMode{
  public:
    enum modes: measureMode_t{
      None      = -1,
      Interrupt = 0,
      Timer1    = 1
    };
};

typedef uint32_t (*IntervalReader)();

class RPMMeasure{
  
  public:
    RPMMeasure(uint8_t interruptPin=2, uint8_t interruptNum=0, uint8_t timer1Pin=8);
    ~RPMMeasure();
    
    void
      update(void),
      setPulsesPerRevolution(uint8_t ppr),
      setMeasureMode(measureMode_t mode),
      setAveragingDepth(uint8_t depth),
      begin(),
      end();
    
    uint16_t
      getRPM();
  
  private:
    uint32_t lastReading;
    uint16_t rpm;
    uint16_t *rpmBuffer = NULL;
    
    uint8_t
      averagingDepth,
      pulsesPerRevolution,
      interruptPin,
      interruptNum,
      timer1Pin,
      bufferHead;
    
    bool begun;
    measureMode_t measureMode;
    
    uint16_t intervalToRPM(uint32_t interval);
    
};

#endif
