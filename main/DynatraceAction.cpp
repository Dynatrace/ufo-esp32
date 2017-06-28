#include "DynatraceMonitoring.h"
#include "DynatraceAction.h"
#include "String.h"
#include <esp_log.h>
#include <time.h>
#include <cJSON.h>


static const char* LOGTAG = "DTAction";


DynatraceAction::DynatraceAction(DynatraceMonitoring* pMon) {
    mpMon = pMon;
    mId = mpMon->getSequence0();
    
	ESP_LOGI(LOGTAG, "Start %i", mId);
}

DynatraceAction::~DynatraceAction() {

}

__uint32_t DynatraceAction::enter(String pName) {
    return this->enter(pName, ACTION_MANUAL, 0);    
};

__uint32_t DynatraceAction::enter(String pName, int pType) {
    return this->enter(pName, pType, 0);    
    
};

__uint32_t DynatraceAction::enter(String pName, __uint32_t pParent) {
    return this->enter(pName, ACTION_MANUAL, pParent);    
    
};

__uint32_t DynatraceAction::enter(String pName, int pType, __uint32_t pParent) {

	ESP_LOGI(LOGTAG, "Start action %s", pName.c_str());
    time_t now;
    time(&now);
    long int ms = 0;
    
    mName = pName;
    mType = pType;
    mParent = pParent;
    mS0 = mpMon->getSequence1();
    mStart = ms;
    return mId;
};

void DynatraceAction::leave() {

    time_t now;
    time(&now);
    long int ms = 0;
    
    mS1 = mpMon->getSequence1();
    mEnd = ms;

    mpMon->addAction(this);

};