/*
 * WiFi.h
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_
#include "sdkconfig.h"
#if defined(CONFIG_WIFI_ENABLED)
#include "esp_wifi.h"
#include "esp_err.h"


#include <string>
#include <vector>

class Config;
class StateDisplay;

class Wifi {

public:
	Wifi();

	void SetConfig(Config* pConfig)						{ mpConfig = pConfig; };
	void SetStateDisplay(StateDisplay* pStateDisplay)  	{ mpStateDisplay = pStateDisplay; };

	void GetLocalAddress(char* sBuf);
	void GetGWAddress(char* sBuf);
	void GetNetmask(char* sBuf);
	void GetMac(__uint8_t uMac[6]);

	void StartAPMode(std::string& rsSsid, std::string& rsPass);
	void StartSTAMode(std::string& rsSsid, std::string& rsPass);
	void StartSTAModeEnterprise(std::string& rsSsid, std::string& rsUser, std::string& rsPass, std::string& rsCA);

	bool IsConnected() { return mbConnected; };
	void addDNSServer(std::string ip);
	struct in_addr getHostByName(std::string hostName);
	void setIPInfo(std::string ip, std::string gw, std::string netmask);

	esp_err_t OnEvent(system_event_t *event);

private:
	void Connect();
	void StartAP();

private:
	Config* mpConfig;
	StateDisplay* mpStateDisplay;

	std::string      ip;
	std::string      gw;
	std::string      netmask;

	__uint8_t        muMode;
	std::string      msSsid;
	std::string      msPass;
	std::string      msUser;
	std::string      msCA;

	__uint8_t 		muConnectedClients;
	bool 			mbConnected;

	int dnsCount=0;
	char *dnsServer = nullptr;
};

#endif // CONFIG_WIFI_ENABLED
#endif /* MAIN_WIFI_H_ */
