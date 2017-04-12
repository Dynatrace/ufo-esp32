#ifndef MAIN_STATEDISPLAY_H_
#define MAIN_STATEDISPLAY_H_

#include "DotstarStripe.h"

#define MODE_None				0
#define MODE_APNotConnected		1
#define MODE_STANotConnected	2
#define MODE_APConnected		3
#define MODE_STAConnected		4


class Wifi;

class StateDisplay {
public:
	StateDisplay();
	virtual ~StateDisplay();

	void SetAPMode(bool b)		{ mbAPMode = b; };
	void SetConnected(bool b, Wifi* pWifi);

	void Display(DotstarStripe& rStripeLevel1, DotstarStripe& rStripeLevel2);

	void StartShowingIp();
	void DisplayIp(DotstarStripe& rStripeLevel1, DotstarStripe& rStripeLevel2);

private:
	bool mbAPMode;
	bool mbConnected;

	__uint8_t muState;
	__uint16_t muStateTimer;

	char msIp[16];
	__uint8_t uPos;
	__uint16_t uTick;
	bool bShortBreak;
	__uint8_t uColor;
__uint8_t uColorValue[3];
};

#endif /* MAIN_STATEDISPLAY_H_ */
