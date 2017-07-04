#ifndef MAIN_UFO_H_
#define MAIN_UFO_H_

#include "DotstarStripe.h"
#include "DisplayCharter.h"
#include "DisplayCharterLogo.h"
#include "StateDisplay.h"
#include "DynatraceIntegration.h"
#include "DynatraceMonitoring.h"
#include "AWSIntegration.h"
#include "Wifi.h"
#include "Config.h"
#include "UfoWebServer.h"
#include "ApiStore.h"

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

	Config& 				GetConfig()			{ return mConfig; };
	Wifi& 					GetWifi()			{ return mWifi; };
	DisplayCharterLogo& 	GetLogoDisplay() 	{ return mDisplayCharterLogo; };
	ApiStore& 				GetApiStore() 		{ return mApiStore; };
	DynatraceIntegration&	GetDtIntegration() 	{ return mDt; };
	AWSIntegration&			GetAWSIntegration() { return mAws; };
    String&					GetId()				{ return mId; };

	DynatraceMonitoring dt;
	
private:

	void SetId();
    String mId;
	
	DisplayCharter mDisplayCharterLevel1;
	DisplayCharter mDisplayCharterLevel2;
	DisplayCharterLogo mDisplayCharterLogo;

	StateDisplay mStateDisplay;

	DotstarStripe mStripeLevel1;
	DotstarStripe mStripeLevel2;
	DotstarStripe mStripeLogo;

	Wifi mWifi;
	UfoWebServer mServer;

	Config mConfig;
	ApiStore mApiStore;

	DynatraceIntegration mDt;
	AWSIntegration mAws;

	bool mbButtonPressed;
	bool mbApiCallReceived;
};

#endif /* MAIN_UFO_H_ */
