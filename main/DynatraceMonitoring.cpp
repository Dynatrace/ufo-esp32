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

__uint32_t DynatraceMonitoring::enterAction() {
    return this->enterAction(ACTION_MANUAL, 0);    
};

__uint32_t DynatraceMonitoring::enterAction(int pType) {
    return this->enterAction(pType, 0);    
};

__uint32_t DynatraceMonitoring::enterAction(__uint32_t pParent) {
    return this->enterAction(ACTION_MANUAL, pParent);
};

__uint32_t DynatraceMonitoring::enterAction(int pType, __uint32_t pParent) {
    __uint32_t id = 0;
    DynatraceAction action(this);
    id = action.enter(pType, pParent);
    return id;
};

long int DynatraceMonitoring::getSequence0() {
    return ++seq0;
};

long int DynatraceMonitoring::getSequence1() {
    return ++seq1;
};

