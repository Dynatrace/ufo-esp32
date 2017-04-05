/*
 * Ufo.cpp
 *
 *  Created on: 29.03.2017
 *      Author: helmut.spiegl
 */

#include "Ufo.h"
#include "DotstarStripe.h"

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD


extern "C"{
	void app_main();
}


void task1_function(void *pvParameter)
{
	((Ufo*)pvParameter)->Task1();
	vTaskDelete(NULL);
}

//----------------------------------------------------------------------------------------


Ufo::Ufo() : mStripeLevel1(15, GPIO_NUM_16, GPIO_NUM_17), mStripeLevel2(15, GPIO_NUM_16, GPIO_NUM_18), mStripeLogo(4, GPIO_NUM_16, GPIO_NUM_19){
}

Ufo::~Ufo() {
}

void Ufo::Start(){

	gpio_pad_select_gpio(10);
	gpio_set_direction(GPIO_NUM_10, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_10, GPIO_PULLUP_ONLY);

	gpio_pad_select_gpio(16);
	gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(17);
	gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(18);
	gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(19);
	gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);

	mStripeLogo.SetLeds(0, 1, 0, 100, 255);
	mStripeLogo.SetLeds(1, 1, 125, 255, 0);
	mStripeLogo.SetLeds(2, 1, 0, 255, 0);
	mStripeLogo.SetLeds(3, 1, 255, 0, 150);

	mStripeLogo.Show();

	xTaskCreate(&task1_function, "Task1", 2048, this, 5, NULL);

	mWifi.connectAP(WIFI_SSID, WIFI_PASS);



}

void Ufo::Task1(){
	/*mStripeLevel1.InitColor(0, 255, 0);
	mStripeLevel1.SetLeds(0, 1, 255, 0, 0);
	mStripeLevel1.SetLeds(1, 1, 255, 0, 0);
	mStripeLevel1.SetLeds(2, 1, 255, 0, 0);

	mStripeLevel1.SetStartPos(i);
	mStripeLevel1.Show();
	i++;
	i = i % mStripeLevel1.getCount();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t i = 0;*/

	while (1){

		if (mWifi.IsConnected()){
			printf("starting server\n");
			mServer.Start(8912);
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);

	}
}


//-----------------------------------------------------------------------------------------

Ufo ufo;

void app_main(){

	nvs_flash_init();

	ufo.Start();
}

