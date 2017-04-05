/*
 * Ufo.h
 *
 *  Created on: 29.03.2017
 *      Author: helmut.spiegl
 */

#ifndef MAIN_UFO_H_
#define MAIN_UFO_H_

#include "DotstarStripe.h"
#include "Wifi.h"
#include "Server.h"


class Ufo {
public:
	Ufo();
	virtual ~Ufo();

	void Start();

	void Task1();

private:
	DotstarStripe mStripeLevel1;
	DotstarStripe mStripeLevel2;
	DotstarStripe mStripeLogo;

	WiFi mWifi;
	Server mServer;
};

#endif /* MAIN_UFO_H_ */
