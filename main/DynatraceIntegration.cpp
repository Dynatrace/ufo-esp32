#include "DynatraceIntegration.h"
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


typedef struct{
    DynatraceIntegration* pIntegration;
    __uint8_t uTaskId;
} TDtTaskParam;

void task_function_dynatrace_integration(void *pvParameter)
{
    ESP_LOGI(LOGTAG, "task_function_dynatrace_integration");
    TDtTaskParam* p = (TDtTaskParam*)pvParameter;
    ESP_LOGI(LOGTAG, "run pIntegration");
	p->pIntegration->Run(p->uTaskId);
    ESP_LOGI(LOGTAG, "Ending Task %d", p->uTaskId);
    delete p;

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


void DynatraceIntegration::Init(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing) {
	ESP_LOGI(LOGTAG, "Init");
    DynatraceAction* dtIntegration = pUfo->dt.enterAction("Init DynatraceIntegration");	

    mpUfo = pUfo;  
    mpDisplayLowerRing = pDisplayLowerRing;
    mpDisplayUpperRing = pDisplayUpperRing;

    mpConfig = &(mpUfo->GetConfig());
    mEnabled = false;
    mInitialized = true;
    mActTaskId = 1;
    mActConfigRevision = 0;
    ProcessConfigChange();
    mpUfo->dt.leaveAction(dtIntegration);
}

// care about starting or ending the task
void DynatraceIntegration::ProcessConfigChange(){
    if (!mInitialized)
        return; 

    //its a little tricky to handle an enable/disable race condition without ending up with 0 or 2 tasks running
    //so whenever there is a (short) disabled situation detected we let the old task go and do not need to wait on its termination
    if (mpConfig->mbDTEnabled){
        if (!mEnabled){
            TDtTaskParam* pParam = new TDtTaskParam;
            pParam->pIntegration = this;
            pParam->uTaskId = mActTaskId;
            ESP_LOGI(LOGTAG, "Create Task %d", mActTaskId);
            xTaskCreate(&task_function_dynatrace_integration, "Task_DynatraceIntegration", 8192, pParam, 5, NULL);
            ESP_LOGI(LOGTAG, "task created");
        }
    }
    else{
        if (mEnabled)
            mActTaskId++;
    }
    mEnabled = mpConfig->mbDTEnabled;
    mActConfigRevision++;
}

void ParseIntegrationUrl(Url& rUrl, String& sEnvIdOrUrl, String& sApiToken){
    String sHelp;
 
    ESP_LOGI(LOGTAG, "%s", sEnvIdOrUrl.c_str());
    ESP_LOGD(LOGTAG, "%s", sApiToken.c_str());

    if (sEnvIdOrUrl.length()){
        if (sEnvIdOrUrl.indexOf(".") < 0){ //an environment id
            sHelp.printf("https://%s.live.dynatrace.com/api/v1/problem/status?Api-Token=%s", sEnvIdOrUrl.c_str(), sApiToken.c_str());
        }
        else{
            if (sEnvIdOrUrl.startsWith("http")){
                if (sEnvIdOrUrl.charAt(sEnvIdOrUrl.length()-1) == '/')
                    sHelp.printf("%sapi/v1/problem/status?Api-Token=%s", sEnvIdOrUrl.c_str(), sApiToken.c_str());
                else
                    sHelp.printf("%s/api/v1/problem/status?Api-Token=%s", sEnvIdOrUrl.c_str(), sApiToken.c_str());
            }
            else{
                if (sEnvIdOrUrl.charAt(sEnvIdOrUrl.length()-1) == '/')
                    sHelp.printf("https://%sapi/v1/problem/status?Api-Token=%s", sEnvIdOrUrl.c_str(), sApiToken.c_str());
                else
                    sHelp.printf("https://%s/api/v1/problem/status?Api-Token=%s", sEnvIdOrUrl.c_str(), sApiToken.c_str());
            }
                
        }   
    }
    
    ESP_LOGD(LOGTAG, "URL: %s", sHelp.c_str());
    rUrl.Clear();
    rUrl.Parse(sHelp);
 }

void DynatraceIntegration::Run(__uint8_t uTaskId) {
    __uint8_t uConfigRevision = mActConfigRevision - 1;
    vTaskDelay(5000 / portTICK_PERIOD_MS);
	ESP_LOGD(LOGTAG, "Run");
    while (1) {
        if (mpUfo->GetWifi().IsConnected()) {
            //Configuration is not atomic - so in case of a change there is the possibility that we use inconsistent credentials - but who cares (the next time it would be fine again)
            if (uConfigRevision != mActConfigRevision){
                uConfigRevision = mActConfigRevision; //memory barrier would be needed here
                ParseIntegrationUrl(mDtUrl, mpConfig->msDTEnvIdOrUrl, mpConfig->msDTApiToken);
            }
            GetData();
            ESP_LOGD(LOGTAG, "free heap after processing DT: %i", esp_get_free_heap_size());            

            for (int i=0 ; i < mpConfig->miDTInterval ; i++){
                vTaskDelay(1000 / portTICK_PERIOD_MS);

                if (uTaskId != mActTaskId)
                    return;
            }
        }
        else
		    vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (uTaskId != mActTaskId)
            return;
    }
}

void DynatraceIntegration::GetData() {
	ESP_LOGD(LOGTAG, "polling");
    DynatraceAction* dtPollApi = mpUfo->dt.enterAction("Poll Dynatrace API");	
    if (dtClient.Prepare(&mDtUrl)) {

        DynatraceAction* dtHttpGet = mpUfo->dt.enterAction("HTTP Get Request", WEBREQUEST, dtPollApi);	
        unsigned short responseCode = dtClient.HttpGet();
        String response = dtClient.GetResponseData();
        mpUfo->dt.leaveAction(dtHttpGet, &mDtUrlString, responseCode, response.length());
        if (responseCode == 200) {
            DynatraceAction* dtProcess = mpUfo->dt.enterAction("Process Dynatrace Metrics", dtPollApi);	
            Process(response);
            mpUfo->dt.leaveAction(dtProcess);
        } else {
            ESP_LOGE(LOGTAG, "Communication with Dynatrace failed - error %u", responseCode);
            DynatraceAction* dtFailure = mpUfo->dt.enterAction("Handle Dynatrace API failure", dtPollApi);	
            HandleFailure();
            mpUfo->dt.leaveAction(dtFailure);
        }        
    }
    dtClient.Clear();
    mpUfo->dt.leaveAction(dtPollApi);
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
          mpDisplayUpperRing->SetLeds(0, 15, 0x00ff00);
          mpDisplayLowerRing->SetLeds(0, 15, 0x00ff00);
          mpDisplayUpperRing->SetMorph(4000, 6);
          mpDisplayLowerRing->SetMorph(4000, 6);
          break;
        case 1:
          mpDisplayUpperRing->SetLeds(0, 15, (miApplicationProblems > 0) ? 0xff0000 : ((miServiceProblems > 0) ? 0xff00aa : 0xffaa00));
          mpDisplayLowerRing->SetLeds(0, 15, (miApplicationProblems > 0) ? 0xff0000 : ((miServiceProblems > 0) ? 0xff00aa : 0xffaa00));
          mpDisplayUpperRing->SetMorph(1000, 8);
          mpDisplayLowerRing->SetMorph(1000, 8);
          break;
        case 2:
          mpDisplayUpperRing->SetLeds(0, 7, (miApplicationProblems > 0) ? 0xff0000 : ((miServiceProblems > 0) ? 0xff00aa : 0xffaa00));
          mpDisplayLowerRing->SetLeds(0, 7, (miApplicationProblems > 0) ? 0xff0000 : ((miServiceProblems > 0) ? 0xff00aa : 0xffaa00));
          mpDisplayUpperRing->SetLeds(8, 6, (miApplicationProblems > 1) ? 0xff0000 : ((miApplicationProblems + miServiceProblems > 1) ? 0xff00aa : 0xffaa00));
          mpDisplayLowerRing->SetLeds(8, 6, (miApplicationProblems > 1) ? 0xff0000 : ((miApplicationProblems + miServiceProblems > 1) ? 0xff00aa : 0xffaa00));
          mpDisplayUpperRing->SetWhirl(180, true);
          mpDisplayLowerRing->SetWhirl(180, true);
          break;
        default:
          mpDisplayUpperRing->SetLeds(0, 4, (miApplicationProblems > 0) ? 0xff0000 : ((miServiceProblems > 0) ? 0xff00aa : 0xffaa00));
          mpDisplayLowerRing->SetLeds(0, 4, (miApplicationProblems > 0) ? 0xff0000 : ((miServiceProblems > 0) ? 0xff00aa : 0xffaa00));
          mpDisplayUpperRing->SetLeds(5, 4, (miApplicationProblems > 1) ? 0xff0000 : ((miApplicationProblems + miServiceProblems > 1) ? 0xff00aa : 0xffaa00));
          mpDisplayLowerRing->SetLeds(5, 4, (miApplicationProblems > 1) ? 0xff0000 : ((miApplicationProblems + miServiceProblems > 1) ? 0xff00aa : 0xffaa00));
          mpDisplayUpperRing->SetLeds(10, 4, (miApplicationProblems > 2) ? 0xff0000 : ((miApplicationProblems + miServiceProblems > 2) ? 0xff00aa : 0xffaa00));
          mpDisplayLowerRing->SetLeds(10, 4, (miApplicationProblems > 2) ? 0xff0000 : ((miApplicationProblems + miServiceProblems > 2) ? 0xff00aa : 0xffaa00));
          mpDisplayUpperRing->SetWhirl(180, true);
          mpDisplayLowerRing->SetWhirl(180, true);
          break;
      }

}


void DynatraceIntegration::Process(String& jsonString) {
    
    cJSON* parentJson = cJSON_Parse(jsonString.c_str());
    if (!parentJson)
        return;
    cJSON* json = cJSON_GetObjectItem(parentJson, "result");
    if (!json)
        return;
    
    if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG){
        char* sJsonPrint = cJSON_Print(json);
	    ESP_LOGD(LOGTAG, "processing %s", sJsonPrint);
        free(sJsonPrint);
    }

//    bool changed = false;
    int iTotalProblems = cJSON_GetObjectItem(json, "totalOpenProblemsCount")->valueint;
    int iInfrastructureProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "INFRASTRUCTURE")->valueint;
    int iApplicationProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "APPLICATION")->valueint;
    int iServiceProblems = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "openProblemCounts"), "SERVICE")->valueint;

    ESP_LOGI(LOGTAG, "open Dynatrace problems: %i", iTotalProblems);
    ESP_LOGI(LOGTAG, "open Infrastructure problems: %i", iInfrastructureProblems);
    ESP_LOGI(LOGTAG, "open Application problems: %i", iApplicationProblems);
    ESP_LOGI(LOGTAG, "open Service problems: %i", iServiceProblems);

    cJSON_Delete(parentJson);

    if (iInfrastructureProblems != miInfrastructureProblems) {
//        changed = true;
        miInfrastructureProblems = iInfrastructureProblems;
    }
    if (iApplicationProblems != miApplicationProblems) {
//        changed = true;
        miApplicationProblems = iApplicationProblems;
    }
    if (iServiceProblems != miServiceProblems) {
//        changed = true;
        miServiceProblems = iServiceProblems;
    }
    miTotalProblems = iTotalProblems;

//    if (changed) {
        DisplayDefault();
//    }

}