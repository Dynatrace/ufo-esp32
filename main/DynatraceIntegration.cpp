#include "DynatraceIntegration.h"
#include "WebClient.h"
#include "Url.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "String.h"
#include <esp_log.h>


DynatraceIntegration::DynatraceIntegration(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing) {

    mpUfo = pUfo;  
    mpDisplayLowerRing = pDisplayLowerRing;
    mpDisplayUpperRing = pDisplayUpperRing;
    mpConfig = &(mpUfo->GetConfig());
	ESP_LOGI("Dynatrace", "Start");

}


DynatraceIntegration::~DynatraceIntegration() {

}


bool DynatraceIntegration::Init() {
    mInitialized = true;
    mActive = true;
    mDtEnvId = mpConfig->msDTEnvId;
    mDtApiToken = mpConfig->msDTApiToken;

    mpDtUrl.Build(true, mDtEnvId+".live.dynatrace.com", 443, "/api/v1/problem/status?Api-Token="+mDtApiToken);

	ESP_LOGI("Dynatrace", "Init");
	ESP_LOGI("Dynatrace", "URL: %s", mpDtUrl.GetUrl().c_str());

    return mInitialized;
}

void DynatraceIntegration::Shutdown() {
    mActive = false;
}


void DynatraceIntegration::Poll() {
	ESP_LOGI("Dynatrace", "polling %s", mpDtUrl.GetUrl().c_str());
//    if (dtClient.Prepare(&mpDtUrl)) {
//        unsigned short usResponseCode = dtClient.HttpGet();
//    }

}
