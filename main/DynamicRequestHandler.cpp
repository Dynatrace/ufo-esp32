#include "DynamicRequestHandler.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "DynatraceAction.h"
#include "esp_system.h"
#include <esp_log.h>
#include "Ota.h"
#include "String.h"
#include "WebClient.h"

static char tag[] = "DynamicRequestHandler";

DynamicRequestHandler::DynamicRequestHandler(Ufo* pUfo, DisplayCharter* pDCLevel1, DisplayCharter* pDCLevel2) {
	mpUfo = pUfo;
	mpDisplayCharterLevel1 = pDCLevel1;
	mpDisplayCharterLevel2 = pDCLevel2;

	mbRestart = false;
}

DynamicRequestHandler::~DynamicRequestHandler() {

}

bool DynamicRequestHandler::HandleApiRequest(std::list<TParam>& params, HttpResponse& rResponse){

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle API Request");	

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
	mpUfo->dt.leaveAction(dtHandleRequest);

	return rResponse.Send(sBody.c_str(), sBody.length());
}

bool DynamicRequestHandler::HandleApiListRequest(std::list<TParam>& params, HttpResponse& rResponse){
    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle API List Request");	
	String sBody;
	mpUfo->GetApiStore().GetApisJson(sBody);
	rResponse.AddHeader(HttpResponse::HeaderContentTypeJson);
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.SetRetCode(200);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send(sBody.c_str(), sBody.length());
}
bool DynamicRequestHandler::HandleApiEditRequest(std::list<TParam>& params, HttpResponse& rResponse){

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle API Edit Request");	
	__uint8_t uId = 0xff;
	const char* sNewApi = NULL;
	bool bDelete = false;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){

		if ((*it).paramName == "apiid")
			uId = strtol((*it).paramValue.c_str(), NULL, 10) - 1;
		else if ((*it).paramName == "apiedit")
			sNewApi = (*it).paramValue.c_str();
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
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send();
}


bool DynamicRequestHandler::HandleInfoRequest(std::list<TParam>& params, HttpResponse& rResponse){

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Info Request");	

	char sHelp[20];
	String sBody;

	sBody.reserve(512);
	sBody.printf("{\"apmode\":\"%s\",", mpUfo->GetConfig().mbAPMode ? "1":"0");
	sBody.printf("\"heap\":\"%d\",", esp_get_free_heap_size());
	sBody.printf("\"ssid\":\"%s\",",  mpUfo->GetConfig().msSTASsid.c_str());
	sBody.printf("\"hostname\":\"%s\",",  mpUfo->GetConfig().msHostname.c_str());
	sBody.printf("\"enterpriseuser\":\"%s\",", mpUfo->GetConfig().msSTAENTUser.c_str());
	sBody.printf("\"sslenabled\":\"%s\",", mpUfo->GetConfig().mbWebServerUseSsl ? "1":"0");
	sBody.printf("\"listenport\":\"%d\",", mpUfo->GetConfig().muWebServerPort);

	if (mpUfo->GetConfig().mbAPMode) {
		sBody.printf("\"lastiptoap\":\"%d.%d.%d.%d\",", IP2STR((ip4_addr*)&(mpUfo->GetConfig().muLastSTAIpAddress)));
	} else {
		sBody += "\"ipaddress\":\"";
		sBody += mpUfo->GetWifi().GetLocalAddress();
		mpUfo->GetWifi().GetGWAddress(sHelp);
		sBody.printf("\",\"ipgateway\":\"%s\",", sHelp);
		mpUfo->GetWifi().GetNetmask(sHelp);
		sBody.printf("\"ipsubnetmask\":\"%s\",", sHelp);

		uint8_t uChannel;
		int8_t iRssi;
		mpUfo->GetWifi().GetApInfo(iRssi, uChannel);
		sBody.printf("\"rssi\":\"%d\",", iRssi);
		sBody.printf("\"channel\":\"%d\",", uChannel);
	}
	mpUfo->GetWifi().GetMac((__uint8_t*)sHelp);

	sBody.printf("\"macaddress\":\"%x:%x:%x:%x:%x:%x\",", sHelp[0], sHelp[1], sHelp[2], sHelp[3], sHelp[4], sHelp[5]);
	sBody.printf("\"firmwareversion\":\"%s\",", FIRMWARE_VERSION);
	sBody.printf("\"ufoid\":\"%s\",", mpUfo->GetConfig().msUfoId.c_str());
	sBody.printf("\"ufoname\":\"%s\",", mpUfo->GetConfig().msUfoName.c_str());
	sBody.printf("\"organization\":\"%s\",", mpUfo->GetConfig().msOrganization.c_str());
	sBody.printf("\"department\":\"%s\",", mpUfo->GetConfig().msDepartment.c_str());
	sBody.printf("\"location\":\"%s\",", mpUfo->GetConfig().msLocation.c_str());
	sBody.printf("\"dtenabled\":\"%u\",", mpUfo->GetConfig().mbDTEnabled);
	sBody.printf("\"dtenvid\":\"%s\",", mpUfo->GetConfig().msDTEnvId.c_str());
	sBody.printf("\"dtapitoken\":\"%s\",", mpUfo->GetConfig().msDTApiToken.c_str());
	sBody.printf("\"dtinterval\":\"%u\",", mpUfo->GetConfig().miDTInterval);
	sBody.printf("\"dtmonitoring\":\"%u\"", mpUfo->GetConfig().mbDTMonitoring);
	sBody += '}';

	rResponse.AddHeader(HttpResponse::HeaderContentTypeJson);
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.SetRetCode(200);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send(sBody.c_str(), sBody.length());
}

bool DynamicRequestHandler::HandleDynatraceIntegrationRequest(std::list<TParam>& params, HttpResponse& rResponse){

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Dynatrace Integration Request");	
	String sEnvId;
	String sApiToken;
	bool bEnabled = false;
	int iInterval = 0;

	String sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){
		if ((*it).paramName == "dtenabled")
			bEnabled = (*it).paramValue;
		else if ((*it).paramName == "dtenvid")
			sEnvId = (*it).paramValue;
		else if ((*it).paramName == "dtapitoken")
			sApiToken = (*it).paramValue;
		else if ((*it).paramName == "dtinterval")
			iInterval = (*it).paramValue.toInt();
		it++;
	}

	mpUfo->GetConfig().mbDTEnabled = bEnabled;
	mpUfo->GetConfig().msDTEnvId = sEnvId;
	mpUfo->GetConfig().msDTApiToken = sApiToken;
	mpUfo->GetConfig().miDTInterval = iInterval;

	if (mpUfo->GetConfig().Write())
		mpUfo->GetDtIntegration().ProcessConfigChange();

	ESP_LOGI(tag, "Dynatrace Integration Saved");

	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.AddHeader("Location: /#!pagedynatraceintegrationsettings");
	rResponse.SetRetCode(302);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send();
}

bool DynamicRequestHandler::HandleDynatraceMonitoringRequest(std::list<TParam>& params, HttpResponse& rResponse){

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Dynatrace Monitoring Request");	
	bool bEnabled = false;

	String sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){
		if ((*it).paramName == "dtmonitoring")
			bEnabled = (*it).paramValue;
		else if ((*it).paramName == "ufoname")
			mpUfo->GetConfig().msUfoName = (*it).paramValue;
		else if ((*it).paramName == "organization")
			mpUfo->GetConfig().msOrganization = (*it).paramValue;
		else if ((*it).paramName == "department")
			mpUfo->GetConfig().msDepartment = (*it).paramValue;
		else if ((*it).paramName == "location")
			mpUfo->GetConfig().msLocation = (*it).paramValue;
			
		it++;
	}

	mpUfo->GetConfig().mbDTMonitoring = bEnabled;

	if (mpUfo->GetConfig().Write()) {
		mpUfo->GetAWSIntegration().ProcessConfigChange();
		mpUfo->dt.ProcessConfigChange();
	}

	ESP_LOGI(tag, "Dynatrace Monitoring Saved");

	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	rResponse.AddHeader("Location: /#!pagedynatracemonitoringsettings");
	rResponse.SetRetCode(302);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send();
}

bool DynamicRequestHandler::HandleConfigRequest(std::list<TParam>& params, HttpResponse& rResponse){

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Config Request");	
	const char* sWifiMode = NULL;
	const char* sWifiSsid = NULL;
	const char* sWifiPass = NULL;
	const char* sWifiEntPass = NULL;
	const char* sWifiEntUser = NULL;
	const char* sWifiEntCA = NULL;
	const char* sWifiHostName = NULL;

	String sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){

		if ((*it).paramName == "wifimode")
			sWifiMode = (*it).paramValue.c_str();
		else if ((*it).paramName == "wifissid")
			sWifiSsid = (*it).paramValue.c_str();
		else if ((*it).paramName == "wifipwd")
			sWifiPass = (*it).paramValue.c_str();
		else if ((*it).paramName == "wifientpwd")
			sWifiEntPass = (*it).paramValue.c_str();
		else if ((*it).paramName == "wifientuser")
			sWifiEntUser = (*it).paramValue.c_str();
		else if ((*it).paramName == "wifientca")
			sWifiEntCA = (*it).paramValue.c_str();
		else if ((*it).paramName == "wifihostname")
			sWifiHostName = (*it).paramValue.c_str();
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
	if (sWifiHostName && !mpUfo->GetConfig().msHostname.equals(sWifiHostName)) {
		mpUfo->GetConfig().msHostname = sWifiHostName;
		bOk = true;
	}
	if (bOk){
		mpUfo->GetConfig().mbAPMode = false;
		mpUfo->GetConfig().Write();
		mbRestart = true;
		sBody = "<html><head><title>SUCCESS - firmware update succeded, rebooting shortly.</title>"
				"<meta http-equiv=\"refresh\" content=\"10; url=/\"></head><body>"
				"<h2>New settings stored, rebooting shortly.</h2></body></html>";
		rResponse.SetRetCode(200);
	}
	else{
		rResponse.AddHeader("Location: /#!pagewifisettings");
		rResponse.SetRetCode(302);
	}

	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send(sBody.c_str(), sBody.length());
}

bool DynamicRequestHandler::HandleSrvConfigRequest(std::list<TParam>& params, HttpResponse& rResponse){
    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Server Config Request");	
	const char* sSslEnabled = NULL;
	const char* sListenPort = NULL;
	const char* sServerCert = NULL;
	const char* sCurrentHost = NULL;

	String sBody;

	std::list<TParam>::iterator it = params.begin();
	while (it != params.end()){
		if ((*it).paramName == "sslenabled")
			sSslEnabled = (*it).paramValue.c_str();
		else if ((*it).paramName == "listenport")
			sListenPort = (*it).paramValue.c_str();
		else if ((*it).paramName == "servercert")
			sServerCert = (*it).paramValue.c_str();
		else if ((*it).paramName == "currenthost")
			sCurrentHost = (*it).paramValue.c_str();
		it++;
	}
	mpUfo->GetConfig().mbWebServerUseSsl = (sSslEnabled != NULL);
	mpUfo->GetConfig().muWebServerPort = atoi(sListenPort);
	mpUfo->GetConfig().msWebServerCert = sServerCert;
	ESP_LOGD(tag, "HandleSrvConfigRequest %d, %d", mpUfo->GetConfig().mbWebServerUseSsl, mpUfo->GetConfig().muWebServerPort);
	mpUfo->GetConfig().Write();
	mbRestart = true;
	
	String newUrl = "/";
	if (sCurrentHost){
		String sHost = sCurrentHost;
		int i = sHost.indexOf(':');
		if (i >= 0)
			sHost = sHost.substring(0, i);
		if (sHost.length()){
			if (sSslEnabled != NULL){
				newUrl = "https://" + sHost;
				if (mpUfo->GetConfig().muWebServerPort && (mpUfo->GetConfig().muWebServerPort != 443)){
					newUrl += ':';
					newUrl += sListenPort;
				}
			}
			else{
				newUrl = "http://" + sHost;
				if (mpUfo->GetConfig().muWebServerPort && (mpUfo->GetConfig().muWebServerPort != 80)){
					newUrl += ':';
					newUrl += sListenPort;
				}
			}
			newUrl += '/';
		}
	}

	sBody = "<html><head><title>SUCCESS - firmware update succeded, rebooting shortly.</title>"
			"<meta http-equiv=\"refresh\" content=\"10; url=";
	sBody += newUrl;
	sBody += "\"></head><body><h2>New settings stored, rebooting shortly.</h2></body></html>";
	rResponse.SetRetCode(200);
	rResponse.AddHeader(HttpResponse::HeaderNoCache);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return rResponse.Send(sBody);
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
    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Firmware Request");	
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
			// {"status":"firmware update initiated.", "url":"https://github.com/Dynatrace/ufo-esp32/raw/master/firmware/ufo-esp32.bin"}
			sBody = "{\"status\":\"firmware update initiated.\", \"url\":\"";
			sBody += OTA_LATEST_FIRMWARE_URL;
			sBody += "\"}";
			response.AddHeader(HttpResponse::HeaderContentTypeJson);
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
	mpUfo->dt.leaveAction(dtHandleRequest);
	return response.Send(sBody.c_str(), sBody.length());
}

bool DynamicRequestHandler::HandleCheckFirmwareRequest(std::list<TParam>& params, HttpResponse& response) {

    DynatraceAction* dtHandleRequest = mpUfo->dt.enterAction("Handle Check Firmware Request");	
	String sBody;
	response.SetRetCode(404); // not found

	Url url;
	url.Parse(OTA_LATEST_FIRMWARE_JSON_URL);

	ESP_LOGD(tag, "Retrieve json from: %s", url.GetUrl().c_str());
	WebClient webClient;
	webClient.Prepare(&url);

	unsigned short statuscode = webClient.HttpGet();
    if (statuscode != 200)
		return false;
	int i = webClient.GetResponseData().indexOf("\"version\":\"");
	if (i <= 0)
		return false;
	String version = webClient.GetResponseData().substring(i + 11);
	i = version.indexOf('"');
	if (i <= 0)
		return false;
	version = version.substring(0, i);

	if (!version.equalsIgnoreCase(FIRMWARE_VERSION)){
		sBody = "{\"newversion\":\"New version available: ";
		sBody += version;
		sBody += "\"}";
	}
	else
		sBody = "{}";
	response.SetRetCode(200);
	mpUfo->dt.leaveAction(dtHandleRequest);
	return response.Send(sBody);
	
}