#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

#include "nvs.h"
#include "String.h"

class Config {
public:
	Config();
	virtual ~Config();

	bool Read();
	bool Write();

	void ToggleAPMode() { mbAPMode = !mbAPMode; };

private:
	bool ReadString(nvs_handle h, const char* sKey, String& rsValue);
	bool ReadBigString(nvs_handle h, const char* sKey, String& rsValue);
	bool ReadBool(nvs_handle h, const char* sKey, bool& rbValue);
	bool ReadUInt(nvs_handle h, const char* sKey, __uint32_t& ruValue);
	bool WriteString(nvs_handle h, const char* sKey, String& rsValue);
	bool WriteBigString(nvs_handle h, const char* sKey, String& rsValue);
	bool WriteBool(nvs_handle h, const char* sKey, bool bValue);
	bool WriteUInt(nvs_handle h, const char* sKey, __uint32_t uValue);

public:
	bool mbAPMode;

	String msAPSsid;
	String msAPPass;
	String msSTASsid;
	String msSTAPass;
	String msSTAENTUser;
	String msSTAENTCA;
	String msHostname;
	String msUfoId;
	String msUfoName;
	String msOrganization;
	String msDepartment;
	String msLocation;
	__uint8_t muWifiMode;
	String msSTAENTCert;
	String msSTAENTKey;

    String msDTEnvIdOrUrl;
    String msDTApiToken;
	bool mbDTEnabled;
    __uint32_t muDTInterval;

	bool mbDTMonitoring;

	bool mbWebServerUseSsl;
	__uint16_t muWebServerPort;
	String msWebServerCert;

	__uint32_t muLastSTAIpAddress;
};

#endif /* MAIN_CONFIG_H_ */
