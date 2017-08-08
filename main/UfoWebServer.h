#ifndef MAIN_UFOWEBSERVER_H_
#define MAIN_UFOWEBSERVER_H_

#include "Ota.h"
#include "freertos/FreeRTOS.h"
#include "openssl/ssl.h"
#include "WebServer.h"

class Ufo;
class DisplayCharter;

class UfoWebServer : public WebServer{
public:
	UfoWebServer();
	virtual ~UfoWebServer();

	bool StartUfoSevrver();

	void SetUfo(Ufo* pUfo) { mpUfo = pUfo; };
	void SetDisplayCharter(DisplayCharter* pDCLevel1, DisplayCharter* pDCLevel2){ mpDisplayCharterLevel1 = pDCLevel1; mpDisplayCharterLevel2 = pDCLevel2; };

	virtual bool HandleRequest(HttpRequestParser& httpParser, HttpResponse& httpResponse);


private:
	Ufo* mpUfo;
	DisplayCharter* mpDisplayCharterLevel1;
	DisplayCharter* mpDisplayCharterLevel2;
	bool mbRestart;

	Ota mOta;

};

#endif 
