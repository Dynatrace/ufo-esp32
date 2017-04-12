#ifndef MAIN_WEBSERVER_H_
#define MAIN_WEBSERVER_H_

#include "freertos/FreeRTOS.h"

class Ufo;
class DisplayCharter;

class WebServer {
public:
	WebServer();
	virtual ~WebServer();

	void SetUfo(Ufo* pUfo) { mpUfo = pUfo; };
	void SetDisplayCharter(DisplayCharter* pDCLevel1, DisplayCharter* pDCLevel2){ mpDisplayCharterLevel1 = pDCLevel1; mpDisplayCharterLevel2 = pDCLevel2; };

	bool Start(__uint16_t port);

	void WebRequestHandler(int socket);

private:
	Ufo* mpUfo;
	DisplayCharter* mpDisplayCharterLevel1;
	DisplayCharter* mpDisplayCharterLevel2;

};

#endif /* MAIN_WEBSERVER_H_ */
