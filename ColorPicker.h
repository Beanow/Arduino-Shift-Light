#ifndef ColorPicker_h
#define ColorPicker_h

#include <FastLED.h>

class ColorPicker
{
    public:
        static CRGB at(uint8_t index, bool allowBlack=false);
        static CRGB next(uint8_t &index, bool allowBlack=false);
        static CRGB prev(uint8_t &index, bool allowBlack=false);
};

#endif
