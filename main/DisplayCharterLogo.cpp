#include <freertos/FreeRTOS.h>
#include "DisplayCharterLogo.h"
#include <esp_log.h>

DisplayCharterLogo::DisplayCharterLogo() {
	Init();
}

DisplayCharterLogo::~DisplayCharterLogo() {
}


void DisplayCharterLogo::Init(){
	mLedRed[0] 	 = 0;
	mLedGreen[0] = 100;
	mLedBlue[0]  = 255;
	mLedRed[1] 	 = 125;
	mLedGreen[1] = 255;
	mLedBlue[1]  = 0;
	mLedRed[2] 	 = 0;
	mLedGreen[2] = 255;
	mLedBlue[2]  = 0;
	mLedRed[3] 	 = 255;
	mLedGreen[3] = 0;
	mLedBlue[3]  = 150;
	mbChanged = true;
	muSendAnywayCount = 0;
}

void DisplayCharterLogo::SetLed(__uint8_t uLed, __uint8_t r, __uint8_t g, __uint8_t b){
	if (uLed <= 4){
		mLedRed[uLed] 	= r;
		mLedGreen[uLed] = g;
		mLedBlue[uLed]  = b;
		mbChanged = true;
	}
}

void DisplayCharterLogo::ParseLogoLedArg(String& argument){
	__uint16_t u=0;
	__uint8_t uLed = 0;

	while (u + 6 <= argument.length()){
		SetLed(uLed, strtol(argument.substring(u + 0, u + 2).c_str(), NULL, 16),
					 strtol(argument.substring(u + 2, u + 4).c_str(), NULL, 16),
					 strtol(argument.substring(u + 4, u + 6).c_str(), NULL, 16));
		uLed++;
		u+= 6;
		if ((u >= argument.length()) || (argument.charAt(u) != '|'))
			break;
		u++;
	}
}

void DisplayCharterLogo::Display(DotstarStripe &dotstar){
	if (mbChanged || !muSendAnywayCount){
		for (__uint8_t u=0 ; u<4 ; u++)
			dotstar.SetLeds(u, 1, mLedRed[u], mLedGreen[u], mLedBlue[u]);
		dotstar.Show();

		mbChanged = false;
	}
	muSendAnywayCount++;
}


