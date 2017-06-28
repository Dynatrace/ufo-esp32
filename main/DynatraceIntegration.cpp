#include "DynatraceIntegration.h"
#include "DynatraceMonitoring.h"
#include "DynatraceAction.h"
#include "WebClient.h"
#include "Url.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "String.h"
#include "esp_system.h"
#include <esp_log.h>
#include <cJSON.h>

static const char* LOGTAG = "Dynatrace";


void task_function_dynatrace_integration(void *pvParameter)
{
	((DynatraceIntegration*)pvParameter)->Connect();
	vTaskDelete(NULL);
}


DynatraceIntegration::DynatraceIntegration() {
    miTotalProblems = -1;
    miApplicationProblems = -1;
    miServiceProblems = -1;
    miInfrastructureProblems = -1;

	ESP_LOGI(LOGTAG, "Start");
}


DynatraceIntegration::~DynatraceIntegration() {

}

bool DynatraceIntegration::Init() {
    return Init(mpUfo, mpDisplayLowerRing, mpDisplayUpperRing);
}

bool DynatraceIntegration::Init(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing) {
	ESP_LOGI(LOGTAG, "Init");
    DynatraceAction dtIntegration = pUfo->dt.enterAction("Init DynatraceIntegration");	

    mpUfo = pUfo;  
    mpDisplayLowerRing = pDisplayLowerRing;
    mpDisplayUpperRing = pDisplayUpperRing;
    mpConfig = &(mpUfo->GetConfig());

    if (!mpConfig->mbDTEnabled) {
        return false;
    }
    
    mInitialized = true;
    mDtEnvId = mpConfig->msDTEnvId;
    mDtApiToken = mpConfig->msDTApiToken;
    
    xTaskCreate(&task_function_dynatrace_integration, "Task_DynatraceIntegration", 8192, this, 5, NULL);
    dtIntegration.leave();
    return mInitialized;            

}

bool DynatraceIntegration::Connect() {
	ESP_LOGI(LOGTAG, "Connect");

    mDtUrl.Build(true, mDtEnvId+".live.dynatrace.com", 443, "/api/v1/problem/status?Api-Token="+mDtApiToken);
    ESP_LOGI(LOGTAG, "URL: %s", mDtUrl.GetUrl().c_str());

    while (1) {
        if (mpUfo->GetWifi().IsConnected()) {
            ESP_LOGI(LOGTAG, "UFO is online");
            mActive = true;
            this->Run();            
        }
		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
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
	ESP_LOGD(LOGTAG, "DisplayDefault: %i", miTotalProblems);
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


bool DynatraceIntegration::Process(String& jsonString) {
    
    cJSON* parentJson = cJSON_Parse(jsonString.c_str());
    cJSON* json = cJSON_GetObjectItem(parentJson, "result");
    bool changed = false;

    int iTotalProblems = cJSON_GetObjectItem(json, "totalOpenProblemsCount")->valueint;
    int iInfrastructureProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "INFRASTRUCTURE")->valueint;
    int iApplicationProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "APPLICATION")->valueint;
    int iServiceProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "SERVICE")->valueint;

    if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG){
        char* sJsonPrint = cJSON_Print(json);
	    ESP_LOGD(LOGTAG, "processing %s", sJsonPrint);
        free(sJsonPrint);
    }
    ESP_LOGI(LOGTAG, "open Dynatrace problems: %i", iTotalProblems);
    ESP_LOGI(LOGTAG, "open Infrastructure problems: %i", iInfrastructureProblems);
    ESP_LOGI(LOGTAG, "open Application problems: %i", iApplicationProblems);
    ESP_LOGI(LOGTAG, "open Service problems: %i", iServiceProblems);

    cJSON_Delete(parentJson);

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

bool DynatraceIntegration::Run() {
	ESP_LOGI(LOGTAG, "Run");
	while (1) {
		if (mpConfig->Changed(&mpConfig->mbDTChanged)) {
			Init();
		}
		if (mActive) {
			GetData();
            ESP_LOGI(LOGTAG, "free heap after processing DT: %i", esp_get_free_heap_size());    
			vTaskDelay((mpConfig->miDTInterval-1) * 1000 / portTICK_PERIOD_MS);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

bool DynatraceIntegration::GetData() {
    if (!mActive) return false;
    if (!mpUfo->GetWifi().IsConnected()) return false;
    DynatraceAction dtPollApi = mpUfo->dt.enterAction("Poll Dynatrace API");	
	ESP_LOGI(LOGTAG, "polling");
    if (dtClient.Prepare(&mDtUrl)) {
        DynatraceAction dtHttpGet = mpUfo->dt.enterAction("HTTP Get Request", WEBREQUEST, &dtPollApi);	
        unsigned short responseCode = dtClient.HttpGet();
        dtHttpGet.leave();
        Process(dtClient.GetResponseData());
        dtClient.Clear();

        if (responseCode != 200) {
            ESP_LOGE(LOGTAG, "Communication with Dynatrace failed - error %u", responseCode);
            this->HandleFailure();
            return false;
        }        
    }
    dtPollApi.leave();
    return true;
}


