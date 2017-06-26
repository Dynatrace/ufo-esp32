#ifndef MAIN_DYNATRACEACTION_H_
#define MAIN_DYNATRACEACTION_H_

#include "String.h"
#include <cJSON.h>


class DynatraceMonitoring;

class DynatraceAction {

public:

    DynatraceAction(DynatraceMonitoring* pMon);
	virtual ~DynatraceAction();
    
    __uint32_t enter();
    __uint32_t enter(int pType);
    __uint32_t enter(__uint32_t pParent);
    __uint32_t enter(int pType, __uint32_t pParent);

private:

    DynatraceMonitoring* mpMon;

    __uint32_t mId;
    __uint32_t mParent;
    String mName;
    uint mType;
    long int mStart;
    __uint32_t mS0;
    long int mEnd;
    __uint32_t mS1;

};

#endif