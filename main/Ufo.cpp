#include <freertos/FreeRTOS.h>
#include "Ufo.h"
#include "DotstarStripe.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <esp_log.h>


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


Ufo::Ufo() : mStripeLevel1(15, GPIO_NUM_16, GPIO_NUM_17), mStripeLevel2(15, GPIO_NUM_16, GPIO_NUM_18), mStripeLogo(4, GPIO_NUM_16, GPIO_NUM_19){
	mServer.SetUfo(this);
	mServer.SetDisplayCharter(&mDisplayCharterLevel1, &mDisplayCharterLevel2);
	mWifi.SetConfig(&mConfig);
	mWifi.SetStateDisplay(&mStateDisplay);
	mbApiCallReceived = false;
}

Ufo::~Ufo() {
}

void Ufo::Start(){
	ESP_LOGD("UFO", "===============================================\n");
	ESP_LOGD("UFO", "Start\n");
	mbButtonPressed = !gpio_get_level(GPIO_NUM_0);
	mConfig.Read();
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

	if (mConfig.mbAPMode){
		if (mConfig.muLastSTAIpAddress){
			char sBuf[16];
			sprintf(sBuf, "%d.%d.%d.%d", IP2STR((ip4_addr*)&mConfig.muLastSTAIpAddress));
			ESP_LOGD("Ufo", "Last IP when connected to AP: %d : %s", mConfig.muLastSTAIpAddress, sBuf);
		}
		mWifi.StartAPMode(mConfig.msAPSsid, mConfig.msAPPass, mConfig.msHostname);
	}
	else{
		if (mConfig.msSTAENTUser.length())
			mWifi.StartSTAModeEnterprise(mConfig.msSTASsid, mConfig.msSTAENTUser, mConfig.msSTAPass, mConfig.msSTAENTCA, mConfig.msHostname);
		else
			mWifi.StartSTAMode(mConfig.msSTASsid, mConfig.msSTAPass, mConfig.msHostname);
	}

}

void Ufo::TaskWebServer(){

	while (1){
		if (mWifi.IsConnected()){
			ESP_LOGD("Ufo", "starting server\n");
			mServer.Start();
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void Ufo::TaskDisplay(){
	while (1){
		if (mWifi.IsConnected() && mbApiCallReceived){
			mDisplayCharterLevel1.Display(mStripeLevel1);
			mDisplayCharterLevel2.Display(mStripeLevel2);
		}
		else
			mStateDisplay.Display(mStripeLevel1, mStripeLevel2);

		mDisplayCharterLogo.Display(mStripeLogo);

		if (!gpio_get_level(GPIO_NUM_0)){
			if (!mbButtonPressed){
				mConfig.ToggleAPMode();
				mConfig.Write();
				esp_restart();
			}
		}
		else
			mbButtonPressed = false;

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


//-----------------------------------------------------------------------------------------

Ufo ufo;

void app_main(){

	nvs_flash_init();
	tcpip_adapter_init();

	ufo.Start();
}

