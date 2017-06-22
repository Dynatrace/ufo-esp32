#include "DynatraceIntegration.h"
#include "WebClient.h"
#include "Url.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "String.h"
#include <esp_log.h>
#include <cJSON.h>

static const char* LOGTAG = "Dynatrace";


DynatraceIntegration::DynatraceIntegration(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing) {

    miTotalProblems = -1;
    miApplicationProblems = -1;
    miServiceProblems = -1;
    miInfrastructureProblems = -1;

    mpUfo = pUfo;  
    mpDisplayLowerRing = pDisplayLowerRing;
    mpDisplayUpperRing = pDisplayUpperRing;
    mpConfig = &(mpUfo->GetConfig());
	ESP_LOGI(LOGTAG, "Start");

}


DynatraceIntegration::~DynatraceIntegration() {

}

bool DynatraceIntegration::OnReceiveBegin(String& sUrl, unsigned int contentLength){
//    ESP_LOGI(LOGTAG, "OnReceiveBegin(%s, %u)", sUrl.c_str(), contentLength);
    return true;
}

bool DynatraceIntegration::OnReceiveBegin(unsigned short int httpStatusCode, bool isContentLength, unsigned int contentLength) {
//    ESP_LOGI(LOGTAG, "OnReceiveBegin(%u, %u)", httpStatusCode, contentLength);
    return true;
}

bool DynatraceIntegration::OnReceiveData(char* buf, int len) {
//    ESP_LOGI(LOGTAG, "OnReceiveData(%d)", len);
    mJson.printf("%s", buf);
//    ESP_LOGI(LOGTAG, "%s", mJson.c_str());
    return true;
}

bool DynatraceIntegration::OnReceiveEnd() {
    ESP_LOGI(LOGTAG, "Data received");
    ESP_LOGI(LOGTAG, "%s", mJson.c_str());
    this->Process();
    return true;
}

bool DynatraceIntegration::Init() {
    mInitialized = true;
    mActive = true;
    mDtEnvId = mpConfig->msDTEnvId;
    mDtApiToken = mpConfig->msDTApiToken;

    mpDtUrl.Build(true, mDtEnvId+".live.dynatrace.com", 443, "/api/v1/problem/status?Api-Token="+mDtApiToken);

	ESP_LOGI(LOGTAG, "Init");
	ESP_LOGI(LOGTAG, "URL: %s", mpDtUrl.GetUrl().c_str());

    return mInitialized;
}

void DynatraceIntegration::HandleFailure() {
    mpDisplayUpperRing->Init();
    mpDisplayLowerRing->Init();
    mpDisplayUpperRing->SetLeds(0, 3, 0x0000ff);
    mpDisplayLowerRing->SetLeds(0, 3, 0x0000ff);
    mpDisplayUpperRing->SetWhirl(220, true);
    mpDisplayLowerRing->SetWhirl(220, false);
    miTotalProblems = -1;
    miApplicationProblems = -1;
    miServiceProblems = -1;
    miInfrastructureProblems = -1;
}


void DynatraceIntegration::DisplayDefault() {
	ESP_LOGI(LOGTAG, "DisplayDefault: %i", miTotalProblems);
    mpDisplayLowerRing->Init();
    mpDisplayUpperRing->Init();

    switch (miTotalProblems){
        case 0:
          mpDisplayUpperRing->SetLeds(0, 15, 0xff0000);
          mpDisplayLowerRing->SetLeds(0, 15, 0xff0000);
          mpDisplayUpperRing->SetMorph(4000, 6);
          mpDisplayLowerRing->SetMorph(4000, 6);
          break;
        case 1:
          mpDisplayUpperRing->SetLeds(0, 15, (miApplicationProblems > 0) ? 0x00ff00 : ((miServiceProblems > 0) ? 0x00ffaa : 0xaaff00));
          mpDisplayLowerRing->SetLeds(0, 15, (miApplicationProblems > 0) ? 0x00ff00 : ((miServiceProblems > 0) ? 0x00ffaa : 0xaaff00));
          mpDisplayUpperRing->SetMorph(1000, 8);
          mpDisplayLowerRing->SetMorph(1000, 8);
          break;
        case 2:
          mpDisplayUpperRing->SetLeds(0, 7, (miApplicationProblems > 0) ? 0x00ff00 : ((miServiceProblems > 0) ? 0x00ffaa : 0xaaff00));
          mpDisplayLowerRing->SetLeds(0, 7, (miApplicationProblems > 0) ? 0x00ff00 : ((miServiceProblems > 0) ? 0x00ffaa : 0xaaff00));
          mpDisplayUpperRing->SetLeds(8, 6, (miApplicationProblems > 1) ? 0x00ff00 : ((miApplicationProblems + miServiceProblems > 1) ? 0x00ffaa : 0xaaff00));
          mpDisplayLowerRing->SetLeds(8, 6, (miApplicationProblems > 1) ? 0x00ff00 : ((miApplicationProblems + miServiceProblems > 1) ? 0x00ffaa : 0xaaff00));
          mpDisplayUpperRing->SetWhirl(180, true);
          mpDisplayLowerRing->SetWhirl(180, true);
          break;
        default:
          mpDisplayUpperRing->SetLeds(0, 4, (miApplicationProblems > 0) ? 0x00ff00 : ((miServiceProblems > 0) ? 0x00ffaa : 0xaaff00));
          mpDisplayLowerRing->SetLeds(0, 4, (miApplicationProblems > 0) ? 0x00ff00 : ((miServiceProblems > 0) ? 0x00ffaa : 0xaaff00));
          mpDisplayUpperRing->SetLeds(5, 4, (miApplicationProblems > 1) ? 0x00ff00 : ((miApplicationProblems + miServiceProblems > 1) ? 0x00ffaa : 0xaaff00));
          mpDisplayLowerRing->SetLeds(5, 4, (miApplicationProblems > 1) ? 0x00ff00 : ((miApplicationProblems + miServiceProblems > 1) ? 0x00ffaa : 0xaaff00));
          mpDisplayUpperRing->SetLeds(10, 4, (miApplicationProblems > 2) ? 0x00ff00 : ((miApplicationProblems + miServiceProblems > 2) ? 0x00ffaa : 0xaaff00));
          mpDisplayLowerRing->SetLeds(10, 4, (miApplicationProblems > 2) ? 0x00ff00 : ((miApplicationProblems + miServiceProblems > 2) ? 0x00ffaa : 0xaaff00));
          mpDisplayUpperRing->SetWhirl(180, true);
          mpDisplayLowerRing->SetWhirl(180, true);
          break;
      }

}


bool DynatraceIntegration::Process() {
    json = cJSON_GetObjectItem(cJSON_Parse(mJson.c_str()), "result");
    bool changed = false;

    int iTotalProblems = cJSON_GetObjectItem(json, "totalOpenProblemsCount")->valueint;
    int iInfrastructureProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "INFRASTRUCTURE")->valueint;
    int iApplicationProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "APPLICATION")->valueint;
    int iServiceProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "SERVICE")->valueint;

	ESP_LOGD(LOGTAG, "processing %s", cJSON_Print(json));
    ESP_LOGI(LOGTAG, "open Dynatrace problems: %i", iTotalProblems);
    ESP_LOGI(LOGTAG, "open Infrastructure problems: %i", iInfrastructureProblems);
    ESP_LOGI(LOGTAG, "open Application problems: %i", iApplicationProblems);
    ESP_LOGI(LOGTAG, "open Service problems: %i", iServiceProblems);

    if (iInfrastructureProblems != miInfrastructureProblems) {
        changed = true;
        miInfrastructureProblems = iInfrastructureProblems;
    }
    if (iApplicationProblems != miApplicationProblems) {
        changed = true;
        miApplicationProblems = iApplicationProblems;
    }
    if (iServiceProblems != miServiceProblems) {
        changed = true;
        miServiceProblems = iServiceProblems;
    }
    miTotalProblems = iTotalProblems;

    if (changed) {
        this->DisplayDefault();
    }

    return changed;
}

void DynatraceIntegration::Shutdown() {
    mActive = false;
}


void task_dynatraceintegration_poll(void *pvParameter) {
	ESP_LOGD(LOGTAG, "Starting DynatraceIntegration Poll Task ....");

	((DynatraceIntegration*)pvParameter)->GetData();
	vTaskDelete(NULL);
}

bool DynatraceIntegration::Poll() {
   	xTaskCreatePinnedToCore(&task_dynatraceintegration_poll, "Task_DynatraceIntegration_Poll", 8192, this, 5, NULL, 0);
    return true;
}

bool DynatraceIntegration::GetData() {
    if (!mActive) return false;
	ESP_LOGI(LOGTAG, "polling %s", mpDtUrl.GetUrl().c_str());
    if (dtClient.Prepare(&mpDtUrl)) {
    	dtClient.SetDownloadHandler(this);

        unsigned short responseCode = dtClient.HttpGet();
        if (responseCode != 200) {
            ESP_LOGE(LOGTAG, "Communication with Dynatrace failed - error %u", responseCode);
            this->HandleFailure();
            return false;
        }        
    }
    dtClient.Clear();
    return true;
}


