#include "ApiStore.h"
#include <freertos/FreeRTOS.h>
#include <esp_log.h>


ApiStore::ApiStore() {
}

ApiStore::~ApiStore() {
}


bool ApiStore::SetApi(__uint8_t uId, std::string& rsApi){
	return false;
}

bool ApiStore::DeleteApi(__uint8_t uId){
	return false;
}

__uint8_t ApiStore::GetApiCount(){
	nvs_handle h;

	if (nvs_open("Apis", NVS_READONLY, &h) != ESP_OK)
		return 0;

	__uint8_t u = 0;
	nvs_get_u8(h, "ApiCount", &u);

	return u;
}

bool ApiStore::GetApi(__uint8_t uId, std::string& rsApi){
	nvs_handle h;

	if (nvs_open("Apis", NVS_READONLY, &h) != ESP_OK)
		return false;

	char sKey[10];
	sprintf(sKey, "Api_%d", uId);
	if (!ReadString(h, sKey, rsApi))
		return nvs_close(h), false;
	return true;
}

//------------------------------------------------------------------------------------------


bool ApiStore::ReadString(nvs_handle h, const char* sKey, std::string& rsValue){
	char* sBuf = NULL;
	__uint32_t u = 0;

	nvs_get_str(h, sKey, NULL, &u);
	if (!u)
		return false;
	sBuf = (char*)malloc(u+1);
	if (nvs_get_str(h, sKey, sBuf, &u) != ESP_OK)
		return false;
	sBuf[u] = 0x00;
	rsValue = sBuf;
	return true;
}


