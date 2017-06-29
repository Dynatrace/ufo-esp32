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
	((DynatraceMonitoring*)pvParameter)->Connect();
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

bool DynatraceMonitoring::Connect() {
	ESP_LOGI(LOGTAG, "Connecting");

    while (!mConnected) {
        if (mpAws->mActive) {
            mConnected = true;
        }
		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return Run();
}

bool DynatraceMonitoring::Run() {
	ESP_LOGI(LOGTAG, "Run");
    mActive = true;
    while (mpAws->mActive) {
        if (mActive) {
    		vTaskDelay(20000 / portTICK_PERIOD_MS);
            mActive = Process();
        }
		vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    Shutdown();
    return mActive;
}

bool DynatraceMonitoring::Process() {

    ESP_LOGI(LOGTAG, "Processing monitoring payload (%i actions)", mActionCount);

    taskENTER_CRITICAL(&myMutex);
    __uint8_t actionCount = mActionCount;
    DynatraceAction* actionBuffer[100];
    for (__uint8_t i=0; i<mActionCount; i++) {
        actionBuffer[i] = mAction[i];
    }
    mActionCount = 0;
    taskEXIT_CRITICAL(&myMutex);

    for (uint i=0; i<actionCount; i++) {
        Send(actionBuffer[i]);
        delete actionBuffer[i];
    }

    return true;
}

void DynatraceMonitoring::Send(DynatraceAction* action) {
    ESP_LOGI(LOGTAG, " - %i: %s", action->getId(), action->getName().c_str());
}

void DynatraceMonitoring::Shutdown() {
	ESP_LOGI(LOGTAG, "Shutdown");
}

DynatraceAction* DynatraceMonitoring::enterAction(String pName) {
    return this->enterAction(pName, ACTION_MANUAL, NULL);    
};

DynatraceAction* DynatraceMonitoring::enterAction(String pName, int pType) {
    return this->enterAction(pName, pType, NULL);    
};

DynatraceAction* DynatraceMonitoring::enterAction(String pName, DynatraceAction* pParent) {
    return this->enterAction(pName, ACTION_MANUAL, pParent);
};

DynatraceAction* DynatraceMonitoring::enterAction(String pName, int pType, DynatraceAction* pParent) {
    __uint32_t id = 0;
    __uint32_t parentId = 0;
    if (pParent) {
        parentId = pParent->getId();
    }
    DynatraceAction* action = new DynatraceAction(this);
    id = action->enter(pName, pType, parentId);
    ESP_LOGD(LOGTAG, "Action %i created: %s", id, pName.c_str());
    return action;
};

void DynatraceMonitoring::addAction(DynatraceAction* action) {
	ESP_LOGI(LOGTAG, "addAction");
    taskENTER_CRITICAL(&myMutex);
    if (mActionCount < 100) {
        ESP_LOGI(LOGTAG, "Action %i added to stack: %s", mActionCount, action->getName().c_str());
        mAction[mActionCount++] = action;
    } else {
        ESP_LOGW(LOGTAG, "Action buffer full, action %s skipped", action->getName().c_str());
        delete action;
    }
    taskEXIT_CRITICAL(&myMutex);
}

__uint32_t DynatraceMonitoring::getSequence0() {
    return seq0++;
};

__uint32_t DynatraceMonitoring::getSequence1() {
    return seq1++;
};

