#ifndef MAIN_DISPLAYCHARTER_H_
#define MAIN_DISPLAYCHARTER_H_

#include "DotstarStripe.h"
#include "String.h"

#define RING_LEDCOUNT 15

class DisplayCharter
{
  public:
    DisplayCharter();
    void Init();
    void SetLeds(__uint8_t pos, __uint8_t count, __uint8_t r, __uint8_t g, __uint8_t b);
    void SetBackground(__uint8_t r, __uint8_t g, __uint8_t b);
    void SetWhirl(__uint8_t wspeed, bool clockwise);
    void SetMorph(__uint16_t period, __uint8_t mspeed);
    __uint16_t ParseLedArg(String& argument, __uint16_t iPos);
    void ParseBgArg(String& argument);
    void ParseWhirlArg(String& argument);
    void ParseMorphArg(String& argument);
    void Display(DotstarStripe &dotstar);

  private:
    //__uint32_t GetPixelColor(__uint8_t i);
    void GetPixelColor(__uint8_t i, __uint8_t& ruR, __uint8_t& ruG, __uint8_t& ruB);
    
  private:
    bool mLedSet[RING_LEDCOUNT];
    __uint8_t mLedRed[RING_LEDCOUNT];
	__uint8_t mLedGreen[RING_LEDCOUNT];
	__uint8_t mLedBlue[RING_LEDCOUNT];
	//__uint32_t ledColor[RING_LEDCOUNT];
	__uint8_t mBackgroundRed;
	__uint8_t mBackgroundGreen;
	__uint8_t mBackgroundBlue;
    //__uint32_t backgroundColor;
    __uint8_t whirlSpeed;
    bool whirlClockwise;
    __uint8_t offset;
    __uint8_t whirlTick;
    __uint8_t morphingState;
    __uint16_t morphPeriod;
    __uint16_t morphPeriodTick;
    __uint8_t morphSpeed;
    __uint8_t morphSpeedTick;
    __uint8_t morphingPercentage;
};


#endif


