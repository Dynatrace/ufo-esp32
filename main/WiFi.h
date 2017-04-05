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


/**
 * @brief %WiFi driver.
 *
 * Encapsulate control of %WiFi functions.
 *
 * Here is an example fragment that illustrates connecting to an access point.
 * @code{.cpp}
 * class TargetWiFiEventHandler: public WiFiEventHandler {
 *    esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
 *       ESP_LOGD(tag, "MyWiFiEventHandler(Class): staGotIp");
 *       // Do something ...
 *       return ESP_OK;
 *    }
 * };
 *
 * WiFi wifi;
 *
 * MyWiFiEventHandler *eventHandler = new MyWiFiEventHandler();
 * wifi.setWifiEventHandler(eventHandler);
 * wifi.connectAP("myssid", "mypassword");
 * @endcode
 */
class WiFi {
private:
	std::string      ip;
	std::string      gw;
	std::string      netmask;

public:
	WiFi();

	bool IsConnected() { return bConnected; };

	void addDNSServer(std::string ip);
	struct in_addr getHostByName(std::string hostName);
	void connectAP(std::string ssid, std::string passwd);
	void dump();
	void startAP(std::string ssid, std::string passwd);
	void setIPInfo(std::string ip, std::string gw, std::string netmask);
	esp_err_t OnEvent(system_event_t *event);


private:
	bool bConnected = false;
	int dnsCount=0;
	char *dnsServer = nullptr;
};

#endif // CONFIG_WIFI_ENABLED
#endif /* MAIN_WIFI_H_ */
