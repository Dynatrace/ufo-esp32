#include "sdkconfig.h"
#if defined(CONFIG_WIFI_ENABLED)

#include "Wifi.h"
#include "Config.h"
#include "StateDisplay.h"
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include "esp_wpa2.h"
#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <tcpip_adapter.h>
#include <string>
#include <string.h>



static char tag[]= "Wifi";


/*
static void setDNSServer(char *ip) {
	ip_addr_t dnsserver;
	ESP_LOGD(tag, "Setting DNS[%d] to %s", 0, ip);
	inet_pton(AF_INET, ip, &dnsserver);
	ESP_LOGD(tag, "ip of DNS is %.8x", *(uint32_t *)&dnsserver);
	dns_setserver(0, &dnsserver);
}
*/


Wifi::Wifi() {
	muConnectedClients = 0;
	mbConnected = false;
}


esp_err_t eventHandler(void *ctx, system_event_t *event) {
	return ((Wifi*)ctx)->OnEvent(event);
}


void Wifi::GetLocalAddress(char* sBuf){
	tcpip_adapter_ip_info_t ip;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
	sprintf(sBuf, "%d.%d.%d.%d", IP2STR(&ip.ip));
}

void Wifi::GetGWAddress(char* sBuf){
	tcpip_adapter_ip_info_t ip;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
	sprintf(sBuf, "%d.%d.%d.%d", IP2STR(&ip.gw));
}
void Wifi::GetNetmask(char* sBuf){
	tcpip_adapter_ip_info_t ip;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
	sprintf(sBuf, "%d.%d.%d.%d", IP2STR(&ip.netmask));
}

void Wifi::GetMac(__uint8_t uMac[6]){
	esp_wifi_get_mac(ESP_IF_WIFI_STA, uMac);
}

void Wifi::StartAPMode(std::string& rsSsid, std::string& rsPass, std::string& rsHostname){
	msSsid = rsSsid;
	msPass = rsPass;
	msHostname = rsHostname;
	StartAP();
}

void Wifi::StartSTAMode(std::string& rsSsid, std::string& rsPass, std::string& rsHostname){
	msSsid = rsSsid;
	msPass = rsPass;
	msHostname = rsHostname;
	Connect();
}

void Wifi::StartSTAModeEnterprise(std::string& rsSsid, std::string& rsUser, std::string& rsPass, std::string& rsCA, std::string& rsHostname)
{
	msSsid = rsSsid;
	msUser = rsUser;
	msPass = rsPass;
	msCA = rsCA;
	msHostname = rsHostname;
	Connect();
}


void Wifi::Connect(){
	ESP_LOGD(tag, "  Connect(<%s><%s><%s><%d>)", msSsid.data(), msUser.data(), msPass.data(), msCA.length());

	nvs_flash_init();
	tcpip_adapter_init();
	if (ip.length() > 0 && gw.length() > 0 && netmask.length() > 0) {
		tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
		tcpip_adapter_ip_info_t ipInfo;
		inet_pton(AF_INET, ip.data(), &ipInfo.ip);
		inet_pton(AF_INET, gw.data(), &ipInfo.gw);
		inet_pton(AF_INET, netmask.data(), &ipInfo.netmask);
		tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	}

	esp_event_loop_init(eventHandler, this);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_STA);
	wifi_config_t config;
	memset(&config, 0, sizeof(config));
	memcpy(config.sta.ssid, msSsid.data(), msSsid.length());
	if (!msUser.length())
		memcpy(config.sta.password, msPass.data(), msPass.length());
	esp_wifi_set_config(WIFI_IF_STA, &config);

	if (msUser.length()){ //Enterprise WPA2
		if (msCA.length())
			esp_wifi_sta_wpa2_ent_set_ca_cert((__uint8_t*)msCA.data(), msCA.length());
		esp_wifi_sta_wpa2_ent_set_identity((__uint8_t*)msUser.data(), msUser.length());
		esp_wifi_sta_wpa2_ent_set_username((__uint8_t*)msUser.data(), msUser.length());
		esp_wifi_sta_wpa2_ent_set_password((__uint8_t*)msPass.data(), msPass.length());
		esp_wifi_sta_wpa2_ent_enable();
	}

	esp_wifi_start();
	ESP_LOGD(tag, "SETTING HOSTNAME: %s", msHostname.c_str() == NULL ? "NULL" : msHostname.c_str());
	ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, msHostname.c_str()));
	esp_wifi_connect();
}


void Wifi::StartAP() {
	ESP_LOGD(tag, "  StartAP(<%s>)", msSsid.data());
	nvs_flash_init();
	tcpip_adapter_init();
	esp_event_loop_init(eventHandler, this);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_AP);
	wifi_config_t config;
	memset(&config, 0, sizeof(config));
	memcpy(config.ap.ssid, msSsid.data(), msSsid.length());
	config.ap.ssid_len = 0;
	memcpy(config.ap.password, msPass.data(), msPass.length());
	config.ap.channel = 0;
	config.ap.authmode = WIFI_AUTH_OPEN;
	config.ap.ssid_hidden = 0;
	config.ap.max_connection = 4;
	config.ap.beacon_interval = 100;
	esp_wifi_set_config(WIFI_IF_AP, &config);
	esp_wifi_start();
}

void Wifi::addDNSServer(std::string ip) {
	ip_addr_t dnsserver;
	ESP_LOGD(tag, "Setting DNS[%d] to %s", dnsCount, ip.c_str());
	inet_pton(AF_INET, ip.c_str(), &dnsserver);
	::dns_setserver(dnsCount, &dnsserver);
	dnsCount++;
}

struct in_addr Wifi::getHostByName(std::string hostName) {
	struct in_addr retAddr;
	struct hostent *he = gethostbyname(hostName.c_str());
	if (he == nullptr) {
		retAddr.s_addr = 0;
		ESP_LOGD(tag, "Unable to resolve %s - %d", hostName.c_str(), h_errno);
	} else {
		retAddr = *(struct in_addr *)(he->h_addr_list[0]);
		//ESP_LOGD(tag, "resolved %s to %.8x", hostName, *(uint32_t *)&retAddr);

	}
	return retAddr;
}

void Wifi::setIPInfo(std::string ip, std::string gw, std::string netmask) {
	this->ip = ip;
	this->gw = gw;
	this->netmask = netmask;
}

esp_err_t Wifi::OnEvent(system_event_t *event){

	esp_err_t rc = ESP_OK;
	switch(event->event_id) {

		case SYSTEM_EVENT_AP_START:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_AP_START");
			mbConnected = true;
			break;
		case SYSTEM_EVENT_AP_STOP:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_AP_STOP");
			mbConnected = false;
			break;
		case SYSTEM_EVENT_AP_STACONNECTED:
			muConnectedClients++;
			ESP_LOGD(tag, "--- SYSTEM_EVENT_AP_STACONNECTED - %d clients", muConnectedClients);
			if (mpStateDisplay)
				mpStateDisplay->SetConnected(true, this);
			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			if (muConnectedClients)
				muConnectedClients--;
			ESP_LOGD(tag, "--- SYSTEM_EVENT_AP_STADISCONNECTED - %d clients", muConnectedClients);
			if (!muConnectedClients && mpStateDisplay)
				mpStateDisplay->SetConnected(false, this);
			break;
		case SYSTEM_EVENT_STA_CONNECTED:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_STA_CONNECTED");
			mbConnected = true;
			if (mpStateDisplay)
				mpStateDisplay->SetConnected(true, this);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_STA_DISCONNECTED");
			mbConnected = false;
			if (mpStateDisplay)
				mpStateDisplay->SetConnected(false, this);
			esp_wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_STA_GOT_IP");
			if (mpStateDisplay)
				mpStateDisplay->SetConnected(true, this);
			tcpip_adapter_ip_info_t ip;
			tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
			if (mpConfig)
				mpConfig->muLastSTAIpAddress = ip.ip.addr;
			break;
		case SYSTEM_EVENT_STA_START:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_STA_START");
			break;
		case SYSTEM_EVENT_STA_STOP:
			ESP_LOGD(tag, "--- SYSTEM_EVENT_STA_STOP");
			break;
		default:
			break;
	}
	return rc;
}



#endif
