#include "DynatraceMonitoring.h"
#include "DynatraceAction.h"
#include "String.h"
#include <esp_log.h>
#include <time.h>
#include <cJSON.h>


static const char* LOGTAG = "DTAction";


DynatraceAction::DynatraceAction(DynatraceMonitoring* pMon) {
    mpMon = pMon;
	ESP_LOGI(LOGTAG, "Start");
}

DynatraceAction::~DynatraceAction() {

}

__uint32_t DynatraceAction::enter() {
    return this->enter(ACTION_MANUAL, 0);    
};

__uint32_t DynatraceAction::enter(int pType) {
    return this->enter(pType, 0);    
    
};

__uint32_t DynatraceAction::enter(__uint32_t pParent) {
    return this->enter(ACTION_MANUAL, pParent);    
    
};

__uint32_t DynatraceAction::enter(int pType, __uint32_t pParent) {

    time_t now;
    time(&now);
    long int ms = 0;
    
    mId = mpMon->getSequence0();
    mType = pType;
    mParent = pParent;
    mS0 = mpMon->getSequence1();
    mStart = ms;
    return mId;
};
