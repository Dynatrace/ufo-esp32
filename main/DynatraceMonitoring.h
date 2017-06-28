#ifndef MAIN_DYNATRACEMONITORING_H_
#define MAIN_DYNATRACEMONITORING_H_

#include "Config.h"
#include "String.h"
#include <cJSON.h>

typedef enum {
    UNKNOWN = 0,
    ACTION_MANUAL = 1,
    ACTION_AUTO = 6,	
    VALUE_STRING = 11,
    VALUE_INT = 12,
    VALUE_DOUBLE = 13,
    NAMED_EVENT = 10,
    SESSION_END = 19, 
    APP_START = 20,	
    DISPLAY = 21,	
    REDISPLAY = 22,	
    WEBREQUEST = 30,
    ERROR_CODE = 40,
    ERROR_STRING = 41,
    EXCEPTION = 42,	
    NS_ERROR = 43,		
    CRASH = 50,			
    CRASH_REPEATED = 51,	
    SESSION_START = 18        
} Action_Types;


class AWSIntegration;
class DynatraceAction;

class DynatraceMonitoring {

public:

    DynatraceMonitoring();
	virtual ~DynatraceMonitoring();
    
    bool Init(AWSIntegration* pAws);
    void Run();
    void Shutdown();
    
    DynatraceAction enterAction(String pName);
    DynatraceAction enterAction(String pName, int pType);
    DynatraceAction enterAction(String pName, DynatraceAction* pParent);
    DynatraceAction enterAction(String pName, int pType, DynatraceAction* pParent);

    void addAction(DynatraceAction* action);

    long int getSequence0();
    long int getSequence1();

    bool mInitialized;
    bool mActive;

private:

    DynatraceAction* mAction[];
    unsigned int mActionCount = 0;

    long int seq0;
    long int seq1;

    AWSIntegration* mpAws;

};

#endif