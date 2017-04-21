#include "DynamicRequestHandler.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "esp_system.h"
#include <esp_log.h>

DynamicRequestHandler::DynamicRequestHandler(Ufo* pUfo, DisplayCharter* pDCLevel1, DisplayCharter* pDCLevel2) {
	mpUfo = pUfo;
	mpDisplayCharterLevel1 = pDCLevel1;
	mpDisplayCharterLevel2 = pDCLevel2;

	mbRestart = false;
}

DynamicRequestHandler::~DynamicRequestHandler() {

}

bool DynamicRequestHandler::HandleApiRequest(std::list<TParam>& params, HttpResponse& rResponse){

	mpUfo->IndicateApiCall();

	std::string sBody;
	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){

		if ((*it).paramName == "top_init")
			mpDisplayCharterLevel1->Init();
		else if ((*it).paramName == "top"){
			__uint16_t i = 0;
			 while (i < (*it).paramValue.length())
				 i = mpDisplayCharterLevel1->ParseLedArg((*it).paramValue, i);
		}
		else if ((*it).paramName == "top_bg")
			mpDisplayCharterLevel1->ParseBgArg((*it).paramValue);
		else if ((*it).paramName == "top_whirl")
			mpDisplayCharterLevel1->ParseWhirlArg((*it).paramValue);
		else if ((*it).paramName == "top_morph")
			mpDisplayCharterLevel1->ParseMorphArg((*it).paramValue);

		if ((*it).paramName == "bottom_init")
			mpDisplayCharterLevel2->Init();
		else if ((*it).paramName == "bottom"){
			__uint16_t i = 0;
			 while (i < (*it).paramValue.length())
				 i = mpDisplayCharterLevel2->ParseLedArg((*it).paramValue, i);
		}
		else if ((*it).paramName == "bottom_bg")
			mpDisplayCharterLevel2->ParseBgArg((*it).paramValue);
		else if ((*it).paramName == "bottom_whirl")
			mpDisplayCharterLevel2->ParseWhirlArg((*it).paramValue);
		else if ((*it).paramName == "bottom_morph")
			mpDisplayCharterLevel2->ParseMorphArg((*it).paramValue);

		else if ((*it).paramName == "logo")
			mpUfo->GetLogoDisplay().ParseLogoLedArg((*it).paramValue);
		else if ((*it).paramName == "logo_reset")
			mpUfo->GetLogoDisplay().Init();


		it++;
	}

	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.data(), sBody.size());
}

bool DynamicRequestHandler::HandleApiListRequest(std::list<TParam>& params, HttpResponse& rResponse){
	std::string sBody;
	mpUfo->GetApiStore().GetApisJson(sBody);
	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.data(), sBody.size());
}
bool DynamicRequestHandler::HandleApiEditRequest(std::list<TParam>& params, HttpResponse& rResponse){

	__uint8_t uId = 0xff;
	const char* sNewApi = NULL;
	bool bDelete = false;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){

		if ((*it).paramName == "apiid")
			uId = strtol((*it).paramValue.data(), NULL, 10) - 1;
		else if ((*it).paramName == "apiedit")
			sNewApi = (*it).paramValue.data();
		else if ((*it).paramName == "delete")
			bDelete = true;
		it++;
	}
	if (bDelete){
		if (!mpUfo->GetApiStore().DeleteApi(uId))
			rResponse.SetRetCode(500);
	}
	else{
		if (!mpUfo->GetApiStore().SetApi(uId, sNewApi))
			rResponse.SetRetCode(500);
	}
	rResponse.AddHeader("Content-Type: text/html");
	rResponse.AddHeader("Location: /");
	rResponse.SetRetCode(302);
	return rResponse.Send();
}


bool DynamicRequestHandler::HandleInfoRequest(std::list<TParam>& params, HttpResponse& rResponse){

	char sBuf[100];
	char sHelp[20];
	std::string sBody;

	sBody = "{";
	sprintf(sBuf, "\"apmode\":\"%d\",", mpUfo->GetConfig().mbAPMode);
	sBody += sBuf;
	sprintf(sBuf, "\"heap\":\"%d Bytes\",", esp_get_free_heap_size());
	sBody += sBuf;
	sprintf(sBuf, "\"ssid\":\"%s\",", mpUfo->GetConfig().msSTASsid.data());
	sBody += sBuf;

	if (mpUfo->GetConfig().mbAPMode){
		sprintf(sBuf, "\"lastiptoap\":\"%d.%d.%d.%d\",", IP2STR((ip4_addr*)&(mpUfo->GetConfig().muLastSTAIpAddress)));
		sBody += sBuf;
	}
	else{
		mpUfo->GetWifi().GetLocalAddress(sHelp);
		sprintf(sBuf, "\"ipaddress\":\"%s\",", sHelp);
		sBody += sBuf;
		mpUfo->GetWifi().GetGWAddress(sHelp);
		sprintf(sBuf, "\"ipgateway\":\"%s\",", sHelp);
		sBody += sBuf;
		mpUfo->GetWifi().GetNetmask(sHelp);
		sprintf(sBuf, "\"ipsubnetmask\":\"%s\",", sHelp);
		sBody += sBuf;
	}
	mpUfo->GetWifi().GetMac((__uint8_t*)sHelp);
	sprintf(sBuf, "\"macaddress\":\"%x:%x:%x:%x:%x:%x\",", sHelp[0], sHelp[1], sHelp[2], sHelp[3], sHelp[4], sHelp[5]);
	sBody += sBuf;
	sprintf(sBuf, "\"firmwareversion\":\"%s\"", FIRMWARE_VERSION);
	sBody += sBuf;
	sBody += '}';

	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.data(), sBody.size());
}

bool DynamicRequestHandler::HandleConfigRequest(std::list<TParam>& params, HttpResponse& rResponse){

	const char* sWifiSsid = NULL;
	const char* sWifiPass = NULL;
	const char* sWifiUser = NULL;
	const char* sWifiCA = NULL;
	std::string sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){

		if ((*it).paramName == "wifissid")
			sWifiSsid = (*it).paramValue.data();
		else if ((*it).paramName == "wifipwd")
			sWifiPass = (*it).paramValue.data();
		else if ((*it).paramName == "wifiuser")
			sWifiUser = (*it).paramValue.data();
		else if ((*it).paramName == "wifica")
			sWifiCA = (*it).paramValue.data();
		it++;
	}
	if (sWifiSsid && sWifiPass){
		mpUfo->GetConfig().msSTASsid = sWifiSsid;
		mpUfo->GetConfig().msSTAPass = sWifiPass;
		if (sWifiUser)
			mpUfo->GetConfig().msSTAENTUser = sWifiUser;
		else
			mpUfo->GetConfig().msSTAENTUser.clear();
		if (sWifiCA){
			//ESP_LOGD("DynamicRequestHandler", "<%s>", sWifiCA);
			mpUfo->GetConfig().msSTAENTCA = sWifiCA;
		}
		else
			mpUfo->GetConfig().msSTAENTCA.clear();
		mpUfo->GetConfig().mbAPMode = false;
		mpUfo->GetConfig().Write();
		mbRestart = true;
		sBody = "Restarting......";

	}

	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.data(), sBody.size());
}

void DynamicRequestHandler::CheckForRestart(){

	if (mbRestart){
		vTaskDelay(100);
		esp_restart();
	}
}





