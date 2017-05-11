#include "DynamicRequestHandler.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "esp_system.h"
#include <esp_log.h>
#include "Ota.h"

#define LATEST_FIRMWARE_URL "https://10.10.29.131:9999/getfirmware"

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

	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.data(), sBody.size());
}

bool DynamicRequestHandler::HandleApiListRequest(std::list<TParam>& params, HttpResponse& rResponse){
	std::string sBody;
	mpUfo->GetApiStore().GetApisJson(sBody);
	rResponse.AddHeader(HttpResponse::HeaderContentTypeJson);
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
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
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
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

	rResponse.AddHeader(HttpResponse::HeaderContentTypeJson);
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.data(), sBody.size());
}

bool DynamicRequestHandler::HandleConfigRequest(std::list<TParam>& params, HttpResponse& rResponse){

	const char* sWifiMode = NULL;
	const char* sWifiSsid = NULL;
	const char* sWifiPass = NULL;
	const char* sWifiEntPass = NULL;
	const char* sWifiEntUser = NULL;
	const char* sWifiEntCA = NULL;
	const char* sWifiHostName = NULL;

	std::string sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){

		if ((*it).paramName == "wifimode")
			sWifiMode = (*it).paramValue.data();
		else if ((*it).paramName == "wifissid")
			sWifiSsid = (*it).paramValue.data();
		else if ((*it).paramName == "wifipwd")
			sWifiPass = (*it).paramValue.data();
		else if ((*it).paramName == "wifientpwd")
			sWifiEntPass = (*it).paramValue.data();
		else if ((*it).paramName == "wifientuser")
			sWifiEntUser = (*it).paramValue.data();
		else if ((*it).paramName == "wifientca")
			sWifiEntCA = (*it).paramValue.data();
		else if ((*it).paramName == "wifihostname")
			sWifiHostName = (*it).paramValue.data();
		it++;
	}

	bool bOk = false;
	if (sWifiSsid){
		mpUfo->GetConfig().msSTASsid = sWifiSsid;

		if (sWifiMode && (sWifiMode[0] == '2')){ //enterprise wap2
			if (sWifiEntUser && (sWifiEntUser[0] != 0x00)){
					mpUfo->GetConfig().msSTAENTUser = sWifiEntUser;
				if (sWifiEntCA)
					mpUfo->GetConfig().msSTAENTCA = sWifiEntCA;
				else
					mpUfo->GetConfig().msSTAENTCA.clear();
				if (sWifiEntPass)
					mpUfo->GetConfig().msSTAPass = sWifiEntPass;
				else
					mpUfo->GetConfig().msSTAPass.clear();
				bOk = true;
			}
		}
		else{
			if (sWifiPass)
				mpUfo->GetConfig().msSTAPass = sWifiPass;
			else
				mpUfo->GetConfig().msSTAPass.clear();
			bOk = true;
		}
	}
	if (sWifiHostName && mpUfo->GetConfig().msHostname.compare(sWifiHostName) != 0) {
		mpUfo->GetConfig().msHostname = sWifiHostName;
		bOk = true;
	}

	if (bOk){
		mpUfo->GetConfig().mbAPMode = false;
		mpUfo->GetConfig().Write();
		mbRestart = true;
		sBody = "Restarting......";
		rResponse.SetRetCode(200);
	}
	else{
		rResponse.AddHeader("Location: /#!pagewifisettings");
		rResponse.SetRetCode(302);
	}

	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	return rResponse.Send(sBody.data(), sBody.size());
}



bool DynamicRequestHandler::HandleFirmwareRequest(std::list<TParam>& params, HttpResponse& response) {
	std::list<TParam>::iterator it = params.begin();
	std::string sBody;
	response.SetRetCode(400); // invalid request
	while (it != params.end()) {
		if ((*it).paramName == "update") {
			Ota ota;
			//if(ota.UpdateFirmware("https://github.com/flyinggorilla/esp32gong/raw/master/firmware/ufo-esp32.bin")) {
			if(ota.UpdateFirmware(LATEST_FIRMWARE_URL)) {
				mbRestart = true;
				sBody = "Firmware update process initiated......";
				response.SetRetCode(200);
			} else {
				//TODO add ota.GetErrorInfo() to inform end-user of problem
				sBody = "Firmware update failed. Rebooting anyway.";
				response.SetRetCode(500);
				mbRestart = true;
			}
		} else if ((*it).paramName == "check") {
			//TODO implement firmware version check;
			sBody = "not implemented";
			response.SetRetCode(501); // not implemented
		} else if ((*it).paramName == "restart") {
			//TODO implement firmware version check;
			sBody = "restarting...";
			mbRestart = true;
			response.SetRetCode(200);
		} else if ((*it).paramName == "switchbootpartition") {
			Ota ota;
			if(ota.SwitchBootPartition()) {
				mbRestart = true;
				sBody = "Switching boot partition successful.";
				response.SetRetCode(200);
			} else {
				//TODO add ota.GetErrorInfo() to inform end-user of problem
				sBody = "Switching boot partition failed.";
				response.SetRetCode(500);
			}
		} else {
				sBody = "Invalid request.";
				response.SetRetCode(400);
		}
		it++;
	}
	response.AddHeader(HttpResponse::HeaderNoCache);
	response.AddHeader(HttpResponse::HeaderContentTypeJson);
	return response.Send(sBody.data(), sBody.size());
}


void DynamicRequestHandler::CheckForRestart(){

	if (mbRestart){
		vTaskDelay(100);
		esp_restart();
	}
}





