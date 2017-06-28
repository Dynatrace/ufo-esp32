#include "Ufo.h"
#include "AWSIntegration.h"
#include "DynatraceMonitoring.h"
#include "DynatraceAction.h"
#include "Config.h"
#include "String.h"
#include <esp_log.h>
#include <cJSON.h>


static const char* LOGTAG = "DTMon";

void task_function_dynatrace_monitoring(void *pvParameter)
{
	((DynatraceMonitoring*)pvParameter)->Run();
	vTaskDelete(NULL);
}


DynatraceMonitoring::DynatraceMonitoring() {
	ESP_LOGI(LOGTAG, "Start");
}

DynatraceMonitoring::~DynatraceMonitoring() {

}

bool DynatraceMonitoring::Init(AWSIntegration* pAws) {
	ESP_LOGI(LOGTAG, "Init");
    mpAws = pAws;      
    mInitialized = true;
    mActive = true;
	xTaskCreate(&task_function_dynatrace_monitoring, "Task_DynatraceMonitoring", 8192, this, 5, NULL);
    return mInitialized;
}

void DynatraceMonitoring::Run() {
	ESP_LOGI(LOGTAG, "Run");
    while (mpAws->mActive) {
        if (mActive) {
        	ESP_LOGI(LOGTAG, "send monitoring payload");
    		vTaskDelay(50000 / portTICK_PERIOD_MS);
        }
		vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    Shutdown();
}

void DynatraceMonitoring::Shutdown() {

}

DynatraceAction DynatraceMonitoring::enterAction(String pName) {
    return this->enterAction(pName, ACTION_MANUAL, NULL);    
};

DynatraceAction DynatraceMonitoring::enterAction(String pName, int pType) {
    return this->enterAction(pName, pType, NULL);    
};

DynatraceAction DynatraceMonitoring::enterAction(String pName, DynatraceAction* pParent) {
    return this->enterAction(pName, ACTION_MANUAL, pParent);
};

DynatraceAction DynatraceMonitoring::enterAction(String pName, int pType, DynatraceAction* pParent) {
    __uint32_t id = 0;
    __uint32_t parentId = 0;
    if (pParent) {
        parentId = pParent->getId();
    }
    DynatraceAction action(this);
    id = action.enter(pName, pType, parentId);
    ESP_LOGD(LOGTAG, "Action %i created: %s", id, pName.c_str());
    return action;
};

void DynatraceMonitoring::addAction(DynatraceAction* action) {
    mAction[mActionCount++] = action;
    ESP_LOGI(LOGTAG, "Action added to stack: %s", action->getName().c_str());
}

long int DynatraceMonitoring::getSequence0() {
    return seq0++;
};

long int DynatraceMonitoring::getSequence1() {
    return seq1++;
};

