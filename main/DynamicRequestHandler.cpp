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

__uint8_t DynamicRequestHandler::HandleApiRequest(std::list<TParam>& params, std::string& body){

	mpUfo->IndicateApiCall();

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

	return 200;
}

__uint8_t DynamicRequestHandler::HandleInfoRequest(std::list<TParam>& params, std::string& body){

	char sBuf[100];
	char sHelp[20];

	body = "{";
	sprintf(sBuf, "\"apmode\":\"%d\",", mpUfo->GetConfig().mbAPMode);
	body += sBuf;
	sprintf(sBuf, "\"heap\":\"%d Bytes\",", esp_get_free_heap_size());
	body += sBuf;
	sprintf(sBuf, "\"ssid\":\"%s\",", mpUfo->GetConfig().msSTASsid.data());
	body += sBuf;

	if (mpUfo->GetConfig().mbAPMode){
		sprintf(sBuf, "\"lastiptoap\":\"%d.%d.%d.%d\",", IP2STR((ip4_addr*)&(mpUfo->GetConfig().muLastSTAIpAddress)));
		body += sBuf;
	}
	else{
		mpUfo->GetWifi().GetLocalAddress(sHelp);
		sprintf(sBuf, "\"ipaddress\":\"%s\",", sHelp);
		body += sBuf;
		mpUfo->GetWifi().GetGWAddress(sHelp);
		sprintf(sBuf, "\"ipgateway\":\"%s\",", sHelp);
		body += sBuf;
		mpUfo->GetWifi().GetNetmask(sHelp);
		sprintf(sBuf, "\"ipsubnetmask\":\"%s\",", sHelp);
		body += sBuf;
	}
	mpUfo->GetWifi().GetMac((__uint8_t*)sHelp);
	sprintf(sBuf, "\"macaddress\":\"%x:%x:%x:%x:%x:%x\",", sHelp[0], sHelp[1], sHelp[2], sHelp[3], sHelp[4], sHelp[5]);
	body += sBuf;
	sprintf(sBuf, "\"firmwareversion\":\"%s\"", FIRMWARE_VERSION);
	body += sBuf;
	body += '}';

	return 200;
}

__uint8_t DynamicRequestHandler::HandleConfigRequest(std::list<TParam>& params, std::string& body){

	const char* sWifiSsid = NULL;
	const char* sWifiPass = NULL;
	const char* sWifiUser = NULL;
	const char* sWifiCA = NULL;

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
		body = "Restarting......";

	}

	return 200;
}

void DynamicRequestHandler::CheckForRestart(){

	if (mbRestart){
		vTaskDelay(100);
		esp_restart();
	}
}





