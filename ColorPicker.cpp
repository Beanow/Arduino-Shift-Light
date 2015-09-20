#include "ColorPicker.h"

#define HUE_STEP 16
#define WHITE_I 16
#define BLACK_I 17

static CHSV makeHSV(uint8_t index){
    if(index == WHITE_I)
        return CHSV(0, 0, 255);
    if(index == BLACK_I)
        return CHSV(0, 0, 0);
    return CHSV(index*HUE_STEP, 255, 255);
}

CRGB ColorPicker::at(uint8_t index, bool allowBlack){
    CRGB color;
    hsv2rgb_rainbow(makeHSV(index), color);
    return color;
}

CRGB ColorPicker::next(uint8_t &index, bool allowBlack){
    index++;
    if(allowBlack && index > BLACK_I)
        index = 0;
    if(!allowBlack && index > WHITE_I)
        index = 0;
    return at(index, allowBlack);
}

CRGB ColorPicker::prev(uint8_t &index, bool allowBlack){
    if(index == 0){
        if(allowBlack)
            index = BLACK_I;
        else
            index = WHITE_I;
    }
    else{
        index--;
    }
    return at(index, allowBlack);
}
