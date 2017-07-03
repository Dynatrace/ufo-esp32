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

    mName = pName;
    mType = pType;
    mParent = pParent;
    mS0 = mpMon->getSequence1();
    mStart = mpMon->getTimestamp();
    return mId;
};

void DynatraceAction::leave() {
    mS1 = mpMon->getSequence1();
    mEnd = mpMon->getTimestamp();
    mpMon->addAction(this);
}

void DynatraceAction::leave(String* pUrl, ushort pResponseCode, uint pResponseSize) {

    mS1 = mpMon->getSequence1();
    mEnd = mpMon->getTimestamp();
    mResponseCode = pResponseCode;
    mResponseSize = pResponseSize;
    mName = *pUrl;

    mpMon->addAction(this);

};

String DynatraceAction::getPayload() {
    String sPayload = "{";
    sPayload.printf("\"name\":\"%s\",", mName.c_str());
    sPayload.printf("\"type\":\"%u\",", mType);
    sPayload.printf("\"id\":\"%u\",", mId);
    sPayload.printf("\"parent\":\"%u\",", mParent);
    sPayload.printf("\"s0\":\"%u\",", mS0);
    sPayload.printf("\"start\":\"%u\",", mStart);
    sPayload.printf("\"t0\":\"%u\",", mStart - mpMon->mStartTimestamp);
    sPayload.printf("\"s1\":\"%u\",", mS1);
    sPayload.printf("\"end\":\"%u\",", mEnd);
    sPayload.printf("\"t1\":\"%u\"", mEnd - mStart);

    if (mType == WEBREQUEST) {
        sPayload.printf(",\"network\": {");
        sPayload.printf("\"responseCode\":\"%u\",", mResponseCode);
        sPayload.printf("\"bytesSent\":\"%u\",", 0);
        sPayload.printf("\"bytesReceived\":\"%u\"", mResponseSize);
        sPayload.printf("}");
    }

    sPayload.printf("}");

    return sPayload;
}

