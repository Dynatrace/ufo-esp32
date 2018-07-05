#include <freertos/FreeRTOS.h>
#include "Ufo.h"
#include "DynatraceIntegration.h"
#include "DynatraceMonitoring.h"
#include "DynatraceAction.h"
#include "AWSIntegration.h"
#include "DotstarStripe.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <esp_log.h>
#include "esp_task_wdt.h"

static const char* LOGTAG = "Ufo";


extern "C"{
	void app_main();
}


void task_function_webserver(void *pvParameter)
{
	((Ufo*)pvParameter)->TaskWebServer();
	vTaskDelete(NULL);
}

void task_function_display(void *pvParameter)
{
	((Ufo*)pvParameter)->TaskDisplay();
	vTaskDelete(NULL);
}


//----------------------------------------------------------------------------------------


Ufo::Ufo() : mStripeLevel1(15, GPIO_NUM_16, GPIO_NUM_18), mStripeLevel2(15, GPIO_NUM_16, GPIO_NUM_17), mStripeLogo(4, GPIO_NUM_16, GPIO_NUM_19){
	mServer.SetUfo(this);
	mServer.SetDisplayCharter(&mDisplayCharterLevel1, &mDisplayCharterLevel2);
	mWifi.SetConfig(&mConfig);
	mWifi.SetStateDisplay(&mStateDisplay);
	mbApiCallReceived = false;
}

Ufo::~Ufo() {
}

void Ufo::Start(){
	ESP_LOGI(LOGTAG, "===================== Dynatrace UFO ========================");
	ESP_LOGI(LOGTAG, "Firmware Version: %s", FIRMWARE_VERSION);
	ESP_LOGI(LOGTAG, "Start");

	esp_task_wdt_init(30, false);

	mConfig.Read();

	DynatraceAction* dtStartup = dt.enterAction("Startup");

	mbButtonPressed = !gpio_get_level(GPIO_NUM_0);
	mStateDisplay.SetAPMode(mConfig.mbAPMode);
	mApiStore.Init();

	gpio_pad_select_gpio(10);
	gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);

	gpio_pad_select_gpio(16);
	gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(17);
	gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(18);
	gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(19);
	gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);

	xTaskCreatePinnedToCore(&task_function_webserver, "Task_WebServer", 12288, this, 5, NULL, 0); //Ota update (upload) just works on core 0
	xTaskCreate(&task_function_display, "Task_Display", 4096, this, 5, NULL);

	// Dynatrace Monitoring
	dt.Init(this, &mAws);

	if (mConfig.mbAPMode){
		if (mConfig.muLastSTAIpAddress){
			char sBuf[16];
			sprintf(sBuf, "%d.%d.%d.%d", IP2STR((ip4_addr*)&mConfig.muLastSTAIpAddress));
			ESP_LOGD(LOGTAG, "Last IP when connected to AP: %d : %s", mConfig.muLastSTAIpAddress, sBuf);
		}
		mWifi.StartAPMode(mConfig.msAPSsid, mConfig.msAPPass, mConfig.msHostname);
		dt.Shutdown();
	}
	else{
		DynatraceAction* dtWifi = dt.enterAction("Start Wifi", dtStartup);	
		switch (mConfig.muWifiMode){
		case 1:
			mWifi.StartSTAMode(mConfig.msSTASsid, mConfig.msSTAPass, mConfig.msHostname);
			break;
		case 2:
			mWifi.StartSTAModeEnterprisePEAP(mConfig.msSTASsid, mConfig.msSTAENTUser, mConfig.msSTAPass, mConfig.msSTAENTCA, mConfig.msHostname);
			break;
		case 3:
			mWifi.StartSTAModeEnterpriseTLS(mConfig.msSTASsid, mConfig.msSTAENTCert, mConfig.msSTAENTKey, mConfig.msSTAENTCA, mConfig.msHostname);
			break;
		}

		dt.leaveAction(dtWifi);
		SetId();
		// Dynatrace API Integration
		mDt.Init(this, &mDisplayCharterLevel1, &mDisplayCharterLevel2);
		// AWS communication layer
		mAws.Init(this);
	}
	dt.leaveAction(dtStartup);

}

void Ufo::TaskWebServer(){

	while (1){
		if (mWifi.IsConnected()){
			ESP_LOGI("Ufo", "starting Webserver");
			mServer.StartUfoServer();
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void Ufo::TaskDisplay(){
	__uint8_t uSendState = 0;

	esp_task_wdt_add(NULL);

	while (1){
		if (mWifi.IsConnected() && (mbApiCallReceived || (mDt.IsActive() && mStateDisplay.IpShownLongEnough()))){
			if (!uSendState){
				mDisplayCharterLevel1.Display(mStripeLevel1, true);
				mDisplayCharterLevel2.Display(mStripeLevel2, true);
				uSendState++;
			}
			else{
				mDisplayCharterLevel1.Display(mStripeLevel1, false);
				mDisplayCharterLevel2.Display(mStripeLevel2, false);
				if (uSendState >= 5)
					uSendState = 0;
				else
					uSendState++;
			}
		}
		else
			mStateDisplay.Display(mStripeLevel1, mStripeLevel2);

		mDisplayCharterLogo.Display(mStripeLogo);

		if (!gpio_get_level(GPIO_NUM_0)){
			if (!mbButtonPressed){
				ESP_LOGI("Ufo", "button pressed");
				mDisplayCharterLevel1.SetLeds(0, 15, 0x440044);
				mDisplayCharterLevel2.SetLeds(0, 15, 0x440044);
				mDisplayCharterLevel1.Display(mStripeLevel1, true);
				mDisplayCharterLevel2.Display(mStripeLevel2, true);
				vTaskDelay(200);
				mConfig.ToggleAPMode();
				mConfig.Write();
				if (mConfig.mbAPMode){
					ESP_LOGI("Ufo", "enter AP mode");
				} else {
					ESP_LOGI("Ufo", "enter standard mode");					
				}
				esp_restart();
			}
		}
		else
			mbButtonPressed = false;

		esp_task_wdt_reset();
		vTaskDelay(1);
	}
}

void Ufo::InitLogoLeds(){
	mStripeLogo.SetLeds(0, 1, 0, 100, 255);
	mStripeLogo.SetLeds(1, 1, 125, 255, 0);
	mStripeLogo.SetLeds(2, 1, 0, 255, 0);
	mStripeLogo.SetLeds(3, 1, 255, 0, 150);
	mStripeLogo.Show();
	mStripeLevel1.SetLeds(0, 1, 0, 100, 255);
	mStripeLevel1.SetLeds(1, 1, 125, 255, 0);
	mStripeLevel1.SetLeds(2, 1, 0, 255, 0);
	mStripeLevel1.SetLeds(3, 1, 255, 0, 150);
	mStripeLevel1.Show();
}

void Ufo::ShowLogoLeds(){
	mStripeLogo.Show();
	mStripeLevel1.Show();
}

void Ufo::SetId() {
	char sHelp[20];
	mWifi.GetMac((__uint8_t*)sHelp);
	mId.printf("ufo-%x%x%x%x%x%x", sHelp[0], sHelp[1], sHelp[2], sHelp[3], sHelp[4], sHelp[5]);
}

//-----------------------------------------------------------------------------------------

Ufo ufo;

void app_main(){

	nvs_flash_init();
	tcpip_adapter_init();

	ufo.Start();
}

