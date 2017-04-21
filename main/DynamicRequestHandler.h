#ifndef MAIN_DYNAMICREQUESTHANDLER_H_
#define MAIN_DYNAMICREQUESTHANDLER_H_

#include "UrlParser.h"
#include "HttpResponse.h"
#include <string>
#include <list>

class Ufo;
class DisplayCharter;

class DynamicRequestHandler {
public:
	DynamicRequestHandler(Ufo* pUfo, DisplayCharter* pDCLevel1, DisplayCharter* pDCLevel2);
	virtual ~DynamicRequestHandler();

	bool HandleApiRequest(std::list<TParam>& params, HttpResponse& rResponse);
	bool HandleApiListRequest(std::list<TParam>& params, HttpResponse& rResponse);
	bool HandleApiEditRequest(std::list<TParam>& params, HttpResponse& rResponse);
	bool HandleInfoRequest(std::list<TParam>& params, HttpResponse& rResponse);
	bool HandleConfigRequest(std::list<TParam>& params, HttpResponse& rResponse);

	void CheckForRestart();

private:
	Ufo* mpUfo;
	DisplayCharter* mpDisplayCharterLevel1;
	DisplayCharter* mpDisplayCharterLevel2;

	bool mbRestart;
};

#endif /* MAIN_DYNAMICREQUESTHANDLER_H_ */
