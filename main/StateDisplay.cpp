#include "StateDisplay.h"
#include "Wifi.h"


#define IPSPEED    400
#define IPBREAK    40


StateDisplay::StateDisplay() {
	mbAPMode = false;
	mbConnected = false;
	muState = 0;
	muStateTimer = 0;
	msIp[0] = 0x00;
	mbFullCycleDone = false;
}

StateDisplay::~StateDisplay() {
}


void StateDisplay::SetConnected(bool b, Wifi* pWifi){
	if (b){
		pWifi->GetLocalAddress(msIp);
		StartShowingIp();
	}
	mbConnected = b;
};


void StateDisplay::Display(DotstarStripe& rStripeLevel1, DotstarStripe& rStripeLevel2){
	//ESP_LOGD("StateDisplay", "Display %d, %d, %d, %d\n", mbAPMode, mbConnected, muState, muStateTimer);
	if (mbAPMode){
		if (mbConnected){
			switch (muState){
				case 0:
					if (muStateTimer)
						muStateTimer--;
					else{
						muStateTimer = 200;
						muState = 1;
						rStripeLevel1.InitColor(0, 25, 25);
						rStripeLevel2.InitColor(0, 25, 25);
					}
					break;
				case 1:
					if (muStateTimer)
						muStateTimer--;
					else{
						muStateTimer = 200;
						muState = 0;
						rStripeLevel1.InitColor(0, 0, 0);
						rStripeLevel2.InitColor(0, 0, 0);
					}
					break;
				default:
					muStateTimer = 0;
					muState = 0;
			}
		}
		else{
			switch (muState){
				case 0:
					if (muStateTimer)
						muStateTimer--;
					else{
						muStateTimer = 200;
						muState = 1;
						rStripeLevel1.InitColor(0, 0, 25);
						rStripeLevel2.InitColor(0, 0, 0);
					}
					break;
				case 1:
					if (muStateTimer)
						muStateTimer--;
					else{
						muStateTimer = 200;
						muState = 0;
						rStripeLevel1.InitColor(0, 0, 0);
						rStripeLevel2.InitColor(0, 0, 25);
					}
					break;
				default:
					muStateTimer = 0;
					muState = 0;
			}
		}

	}
	else{
		if (mbConnected){
			DisplayIp(rStripeLevel1, rStripeLevel2);
		}
		else{
			switch (muState){
				case 0:
					if (muStateTimer)
						muStateTimer--;
					else{
						muStateTimer = 40;
						muState = 1;
						rStripeLevel1.InitColor(25, 20, 0);
						rStripeLevel2.InitColor(25, 20, 0);
					}
					break;
				case 1:
					if (muStateTimer)
						muStateTimer--;
					else{
						muStateTimer = 200;
						muState = 0;
						rStripeLevel1.InitColor(0, 0, 0);
						rStripeLevel2.InitColor(0, 0, 0);
					}
					break;
				default:
					muStateTimer = 0;
					muState = 0;
			}
		}
	}
	rStripeLevel1.Show();
	rStripeLevel2.Show();
}

void StateDisplay::StartShowingIp(){
	uPos = 0;
	uColor = 0;
	uColorValue[0] = 0xFF;
	uColorValue[1] = 0x00;
	uColorValue[2] = 0x00;
	mbFullCycleDone = false;
	uTick = IPSPEED;
	bShortBreak = false;
}

void StateDisplay::DisplayIp(DotstarStripe& rStripeLevel1, DotstarStripe& rStripeLevel2){
	if (!--uTick){
		if (bShortBreak){
			rStripeLevel1.InitColor(0x30, 0x30, 0x30);
			rStripeLevel2.InitColor(0x30, 0x30, 0x30);
			rStripeLevel1.Show();
			rStripeLevel2.Show();
			uTick = IPBREAK;
			bShortBreak = false;
			return;
		}
		bShortBreak = true;
		uTick = IPSPEED;

		if (!msIp[uPos]){
			uPos = 0;
			if (++uColor >= 3){
				uColor = 0;
				mbFullCycleDone = true;
			}
			switch (uColor){
				case 0:
					uColorValue[0] = 0xFF;
					uColorValue[1] = 0x00;
					uColorValue[2] = 0x00;
					break;
				case 1:
					uColorValue[0] = 0x00;
					uColorValue[1] = 0xFF;
					uColorValue[2] = 0x00;
					break;
				case 2:
					uColorValue[0] = 0x00;
					uColorValue[1] = 0x00;
					uColorValue[2] = 0xFF;
					break;
			}
		}
		rStripeLevel1.InitColor(0, 0, 0);
		rStripeLevel2.InitColor(0, 0, 0);
		char c = msIp[uPos];
		uPos++;

		switch (c){
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
				rStripeLevel1.SetLeds(0, c - '0', uColorValue[0], uColorValue[1], uColorValue[2]);
				rStripeLevel2.SetLeds(0, c - '0', uColorValue[0], uColorValue[1], uColorValue[2]);
				break;
			case '6':
			case '7':
			case '8':
			case '9':
				rStripeLevel1.SetLeds(0, 5, uColorValue[0], uColorValue[1], uColorValue[2]);
				rStripeLevel1.SetLeds(8, c - '5', uColorValue[0], uColorValue[1], uColorValue[2]);
				rStripeLevel2.SetLeds(0, 5, uColorValue[0], uColorValue[1], uColorValue[2]);
				rStripeLevel2.SetLeds(8, c - '5', uColorValue[0], uColorValue[1], uColorValue[2]);
				break;
			case '.':
				rStripeLevel1.SetLeds(0, 1, 0xb0, 0xb0, 0xb0);
				rStripeLevel1.SetLeds(5, 1, 0xb0, 0xb0, 0xb0);
				rStripeLevel1.SetLeds(10, 1, 0xb0, 0xb0, 0xb0);
				rStripeLevel2.SetLeds(0, 1, 0xb0, 0xb0, 0xb0);
				rStripeLevel2.SetLeds(5, 1, 0xb0, 0xb0, 0xb0);
				rStripeLevel2.SetLeds(10, 1, 0xb0, 0xb0, 0xb0);
				break;
		}
		rStripeLevel1.Show();
		rStripeLevel2.Show();
	}
}

