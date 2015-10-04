//RPM sensor.
#define RPM_INT_PIN 2 // interrupt based pin.
#define RPM_INT_NUM 0 // interrupt number for this pin (see attachInterrupt()).
#define RPM_TI1_PIN 8 // timer1 pin.
#define BUFFER_DEPTH 5 // how many values to store in averaging buffer if enabled.

//LCD pins.
#define LCDCLK 12
#define LCDDIO 11

//Button pins.
#define BTN_LEFT 10
#define BTN_UP 6
#define BTN_RIGHT 7
#define BTN_DOWN 9

//RTC module pins.
#define RTC_CE 5
#define RTC_IO 4
#define RTC_SCLK 3

//RGB Pixels.
#define PIXELPIN 13
#define NUMPIXELS 16
#define PIXELTYPE WS2812B // see FastLED library.
#define MAX_POWER 600 // in mW.

//Animation constants.
#define ANIM_SLOW  700
#define ANIM_MED   500
#define ANIM_FAST  300
#define ANIM_TURBO  75

//Normal interval.
#define UPDATE_INTERVAL 10

//Power off settings.
#define POWER_OFF_TIMEOUT 3000
#define DISPLAY_OFF_BRIGHTNESS 2
#define ADDITIONAL_INTERVAL 90

//Some absolute minimum and maximum values.
#define MAX_RPM_SETTING 9900
#define MIN_RPM_SETTING 1000
#define MAX_PROFILES 5

//A load of default values.
#define DEFAULT_PPR 4 // pulses per revolution.
#define DEFAULT_RPM_STEP 20 // the RPM resolution step for the LCD display.
#define DEFAULT_RPM_ANIMATION 0 // left-to-right.
#define DEFAULT_PIXEL_BRIGHTNESS 3
#define DEFAULT_DISPLAY_BRIGHTNESS 3
#define DEFAULT_RPM_MEASUREMODE 1 // timer1 mode.
#define DEFAULT_RPM_BUFFER 1 // yes, use buffer.
#define DEFAULT_RPM_STATIONARY 2500
#define DEFAULT_RPM_LOW 4000
#define DEFAULT_RPM_ACTIVATION 4500
#define DEFAULT_RPM_SHIFT 6500

#define DEFAULT_CLOW 2 // orange.
#define DEFAULT_CPART1 6 // green.
#define DEFAULT_CPART2 4 // yellow.
#define DEFAULT_CPART3 0 // red.
#define DEFAULT_CFLASH1 0 // red.
#define DEFAULT_CFLASH2 17 // black.

#define DEFAULT_SEG1 9
#define DEFAULT_SEG2 5
