#ifndef MAIN_UFO_H_
#define MAIN_UFO_H_

#include "DotstarStripe.h"
#include "DisplayCharter.h"
#include "DisplayCharterLogo.h"
#include "StateDisplay.h"
#include "Wifi.h"
#include "Config.h"
#include "WebServer.h"

#define FIRMWARE_VERSION __DATE__ " - " __TIME__

class Ufo {
public:
	Ufo();
	virtual ~Ufo();

	void Start();

	void TaskWebServer();
	void TaskDisplay();

	void InitLogoLeds();
	void ShowLogoLeds();

	void IndicateApiCall() 	{ mbApiCallReceived = true; };
	Config& GetConfig()		{ return mConfig; };
	Wifi& GetWifi()			{ return mWifi; };
	DisplayCharterLogo& GetLogoDisplay() { return mDisplayCharterLogo; };

private:
	DisplayCharter mDisplayCharterLevel1;
	DisplayCharter mDisplayCharterLevel2;
	DisplayCharterLogo mDisplayCharterLogo;

	StateDisplay mStateDisplay;

	DotstarStripe mStripeLevel1;
	DotstarStripe mStripeLevel2;
	DotstarStripe mStripeLogo;

	Wifi mWifi;
	WebServer mServer;

	Config mConfig;

	bool mbButtonPressed;
	bool mbApiCallReceived;
};

#endif /* MAIN_UFO_H_ */
