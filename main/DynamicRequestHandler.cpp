#include "DynamicRequestHandler.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "esp_system.h"
#include <esp_log.h>
#include "Ota.h"
#include "String.h"

static char tag[] = "WebServer";

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

	String sBody;
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
	return rResponse.Send(sBody.c_str(), sBody.length());
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
	String sBody;

	sBody.reserve(512);
	sBody  = "{\"apmode\":\"";
	sBody += mpUfo->GetConfig().mbAPMode;
	sBody += "\",\"heap\":\"";
	sBody += esp_get_free_heap_size();
	sBody += "\",\"ssid\":\"";
	sBody += mpUfo->GetConfig().msSTASsid.data();
	sBody += "\",\"hostname\":\"";
	sBody += mpUfo->GetConfig().msHostname.data();
	sBody += "\",";

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
	sBody += ",\"dtenabled\":\"";
	sBody += mpUfo->GetConfig().mbDTEnabled;
	sBody += "\",\"dtenvid\":\"";
	sBody += mpUfo->GetConfig().msDTEnvId.data();
	sBody += "\",\"dtapitoken\":\"";
	sBody += mpUfo->GetConfig().msDTApiToken.data();
	sBody += "\",\"dtinterval\":\"";
	sBody += mpUfo->GetConfig().miDTInterval;
	sBody += "\"}";

	rResponse.AddHeader(HttpResponse::HeaderContentTypeJson);
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.SetRetCode(200);
	return rResponse.Send(sBody.c_str(), sBody.length());
}

bool DynamicRequestHandler::HandleDynatraceIntegrationRequest(std::list<TParam>& params, HttpResponse& rResponse){

	const char* sEnvId = NULL;
	const char* sApiToken = NULL;
	bool bEnabled = false;
	int iInterval = 0;

	String sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){
		if ((*it).paramName == "dynatrace-on")
			bEnabled = (*it).paramValue.data();
		else if ((*it).paramName == "dynatrace-environmentid")
			sEnvId = (*it).paramValue.data();
		else if ((*it).paramName == "dynatrace-apitoken")
			sApiToken = (*it).paramValue.data();
		else if ((*it).paramName == "dynatrace-interval")
			iInterval = atoi((*it).paramValue.data());
		it++;
	}

	mpUfo->GetConfig().mbDTEnabled = bEnabled;
	if (sEnvId) mpUfo->GetConfig().msDTEnvId = sEnvId;
	else 		mpUfo->GetConfig().msDTEnvId.clear();
	if (sApiToken) mpUfo->GetConfig().msDTApiToken = sApiToken;
	else 		mpUfo->GetConfig().msDTApiToken.clear();
	mpUfo->GetConfig().miDTInterval = iInterval;

	mpUfo->GetConfig().Write();

	ESP_LOGI(tag, "Dynatrace Integration Saved");

	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.AddHeader("Location: /#!pagedynatraceintegrationsettings");
	rResponse.SetRetCode(302);
	return rResponse.Send();
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
			mpUfo->GetConfig().msSTAENTUser.clear();
			mpUfo->GetConfig().msSTAENTCA.clear();
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

/*
GET: /firmware?update

GET: /firmware?progress

Response:

{ "session": "9724987887789", 
"progress": "22",
"status": "inprogress" }

Session: 32bit unsigned int ID that changes when UFO reboots
Progress: 0..100%
Status: notyetstarted | inprogress | connectionerror | flasherror | finishedsuccess

notyetstarted: Firmware update process has not started.

inprogress: Firmware update is in progress.

connectionerror: Firmware could not be downloaded. 

flasherror: Firmware could not be flashed.

finishedsuccess: Firmware successfully updated. Rebooting now.
*/

bool DynamicRequestHandler::HandleFirmwareRequest(std::list<TParam>& params, HttpResponse& response) {
	std::list<TParam>::iterator it = params.begin();
	String sBody;
	response.SetRetCode(400); // invalid request
	while (it != params.end()) {

		if ((*it).paramName == "progress") {
			short progressPct = 0;
			const char* progressStatus = "notyetstarted";
			int   progress = Ota::GetProgress();
			if (progress >= 0) {
				progressPct = progress;
				progressStatus = "inprogress";
			} else {
				switch (progress) {
					case OTA_PROGRESS_NOTYETSTARTED: progressStatus = "notyetstarted"; 
							break;
					case OTA_PROGRESS_CONNECTIONERROR: progressStatus = "connectionerror"; 
							break;
					case OTA_PROGRESS_FLASHERROR: progressStatus = "flasherror"; 
							break;
					case OTA_PROGRESS_FINISHEDSUCCESS: progressStatus = "finishedsuccess"; 
							progressPct = 100;
							break;
				}
			}
			sBody = "{ \"session\": \"";
			sBody += Ota::GetTimestamp();
			sBody += "\", \"progress\": \"";
			sBody += progressPct;
			sBody += "\", \"status\": \"";
			sBody += progressStatus;
			sBody += "\"}";
			response.AddHeader(HttpResponse::HeaderContentTypeJson);
			response.SetRetCode(200);
		} else if ((*it).paramName == "update") {
			if (Ota::GetProgress() == OTA_PROGRESS_NOTYETSTARTED) {
				Ota::StartUpdateFirmwareTask();
				//TODO implement firmware version check;
			}
			sBody = "<html><head><title>Firmware update progress</title>"
					"<meta http-equiv=\"refresh\" content=\"5; url=/firmware?progresspage\"></head><body>"
					"<h1>Firmware update task initiated....</h1></body></html>";
			response.AddHeader(HttpResponse::HeaderContentTypeHtml);
			response.SetRetCode(200);
		} else if ((*it).paramName == "progresspage") {
			if (Ota::GetProgress() == OTA_PROGRESS_FINISHEDSUCCESS) {
				sBody = "<html><head><title>SUCCESS - firmware update succeded, rebooting shortly.</title>"
				        "<meta http-equiv=\"refresh\" content=\"20; url=/\"></head><body><h1>Progress: ";
			} else {
				sBody = "<html><head><title>Firmware update progress</title>"
						"<meta http-equiv=\"refresh\" content=\"5\"></head><body><h1>Progress: ";
				char buf[64];
				sprintf(buf, "%d%%", Ota::GetProgress());
				sBody += buf;
			}
			sBody += "</h1></body><html>";
			response.AddHeader(HttpResponse::HeaderContentTypeHtml);
			response.SetRetCode(200);
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
	return response.Send(sBody.c_str(), sBody.length());
}


void DynamicRequestHandler::CheckForRestart(){

	if (mbRestart){
		vTaskDelay(100);
		esp_restart();
	}
}





