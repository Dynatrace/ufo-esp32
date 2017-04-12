#ifndef MAIN_DYNAMICREQUESTHANDLER_H_
#define MAIN_DYNAMICREQUESTHANDLER_H_

#include "UrlParser.h"
#include <string>
#include <list>

class Ufo;
class DisplayCharter;

class DynamicRequestHandler {
public:
	DynamicRequestHandler(Ufo* pUfo, DisplayCharter* pDCLevel1, DisplayCharter* pDCLevel2);
	virtual ~DynamicRequestHandler();

	__uint8_t HandleApiRequest(std::list<TParam>& params, std::string& body);
	__uint8_t HandleInfoRequest(std::list<TParam>& params, std::string& body);
	__uint8_t HandleConfigRequest(std::list<TParam>& params, std::string& body);

	void CheckForRestart();

private:
	Ufo* mpUfo;
	DisplayCharter* mpDisplayCharterLevel1;
	DisplayCharter* mpDisplayCharterLevel2;

	bool mbRestart;
};

#endif /* MAIN_DYNAMICREQUESTHANDLER_H_ */
