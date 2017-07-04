#include <freertos/FreeRTOS.h>
#include "Config.h"
#include "nvs_flash.h"
#include <esp_log.h>


#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD


Config::Config() {
	mbAPMode = true;
	msAPSsid = "UFO";
	msHostname = "UFO";

	msSTASsid = WIFI_SSID;
	msSTAPass = WIFI_PASS;

	mbWebServerUseSsl = false;
	muWebServerPort = 0;

	mbDTEnabled = false;

	muLastSTAIpAddress = 0;
}

Config::~Config() {
}

bool Config::Read(){
	nvs_handle h;

	if (nvs_flash_init() != ESP_OK)
		return false;
	if (nvs_open("Ufo Config", NVS_READONLY, &h) != ESP_OK)
		return false;
	ReadBool(h, "APMode", mbAPMode);
	ReadString(h, "APSsid", msAPSsid);
	ReadString(h, "APPass", msAPPass);
	nvs_get_u32(h, "STAIpAddress", &muLastSTAIpAddress);
	ReadString(h, "STASsid", msSTASsid);
	ReadString(h, "STAPass", msSTAPass);
	ReadString(h, "STAENTUser", msSTAENTUser);
	ReadString(h, "STAENTCA", msSTAENTCA);
	ReadString(h, "hostname", msHostname);
	ReadBool(h, "DTEnabled", mbDTEnabled);
	ReadString(h, "DTEnvId", msDTEnvId);
	ReadString(h, "DTApiToken", msDTApiToken);
	ReadInt(h, "DTInterval", miDTInterval);
	ReadBool(h, "DTMonitoring", mbDTMonitoring);
	ReadBool(h, "SrvSSLEnabled", mbWebServerUseSsl);
	nvs_get_u16(h, "SrvListenPort", &muWebServerPort);
	ReadString(h, "SrvCert", msWebServerCert);

	nvs_close(h);
	return true;
}


bool Config::Write()
{
	nvs_handle h;

	if (nvs_flash_init() != ESP_OK)
		return false;
	if (nvs_open("Ufo Config", NVS_READWRITE, &h) != ESP_OK)
		return false;
	nvs_erase_all(h); //otherwise I need double the space

	if (!WriteBool(h, "APMode", mbAPMode))
		return nvs_close(h), false;
	if (!WriteString(h, "APSsid", msAPSsid))
		return nvs_close(h), false;
	if (!WriteString(h, "APPass", msAPPass))
		return nvs_close(h), false;
	if (!WriteString(h, "STASsid", msSTASsid))
		return nvs_close(h), false;
	if (!WriteString(h, "STAPass", msSTAPass))
		return nvs_close(h), false;
	if (!WriteString(h, "hostname", msHostname))
		return nvs_close(h), false;
	if (!WriteString(h, "STAENTUser", msSTAENTUser))
		return nvs_close(h), false;
	if (!WriteString(h, "STAENTCA", msSTAENTCA))
		return nvs_close(h), false;
	if (nvs_set_u32(h, "STAIpAddress", muLastSTAIpAddress) != ESP_OK)
		return nvs_close(h), false;

	if (!WriteBool(h, "DTEnabled", mbDTEnabled))
		return nvs_close(h), false;
	if (!WriteString(h, "DTEnvId", msDTEnvId))
		return nvs_close(h), false;
	if (!WriteString(h, "DTApiToken", msDTApiToken))
		return nvs_close(h), false;
	if (!WriteInt(h, "DTInterval", miDTInterval))
		return nvs_close(h), false;

	if (!WriteBool(h, "DTMonitoring", mbDTMonitoring))
		return nvs_close(h), false;

	if (!WriteBool(h, "SrvSSLEnabled", mbWebServerUseSsl))	
		return nvs_close(h), false;
	if (nvs_set_u16(h, "SrvListenPort", muWebServerPort) != ESP_OK)
		return nvs_close(h), false;
	if (!WriteString(h, "SrvCert", msWebServerCert))
		return nvs_close(h), false;


	nvs_commit(h);
	nvs_close(h);
	return true;
}

//------------------------------------------------------------------------------------

bool Config::ReadString(nvs_handle h, const char* sKey, String& rsValue){
	char* sBuf = NULL;
	__uint32_t u = 0;

	nvs_get_str(h, sKey, NULL, &u);
	if (!u)
		return false;
	sBuf = (char*)malloc(u+1);
	if (nvs_get_str(h, sKey, sBuf, &u) != ESP_OK)
		return free(sBuf), false;
	sBuf[u] = 0x00;
	rsValue = sBuf;
	free(sBuf);
	return true;
}

bool Config::ReadBool(nvs_handle h, const char* sKey, bool& rbValue){
	__uint8_t u;
	if (nvs_get_u8(h, sKey, &u) != ESP_OK)
		return false;
	rbValue = u ? true : false;
	return true;
}

bool Config::ReadInt(nvs_handle h, const char* sKey, int& riValue){
	__uint32_t u;
	if (nvs_get_u32(h, sKey, &u) != ESP_OK)
		return false;
	riValue = u;
	return true;
}

bool Config:: WriteString(nvs_handle h, const char* sKey, String& rsValue){
	esp_err_t err = nvs_set_str(h, sKey, rsValue.c_str());
	if (err != ESP_OK){
		ESP_LOGE("Config", "  <%s> -> %d", sKey, err);
		return false;
	}
	return true;
}


bool Config:: WriteBool(nvs_handle h, const char* sKey, bool bValue){
	return (nvs_set_u8(h, sKey, bValue ? 1 : 0) == ESP_OK);
}

bool Config:: WriteInt(nvs_handle h, const char* sKey, int iValue){
	return (nvs_set_u32(h, sKey, iValue) == ESP_OK);
}
