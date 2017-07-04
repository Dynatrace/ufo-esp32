#ifndef MAIN_DYNATRACEMONITORING_H_
#define MAIN_DYNATRACEMONITORING_H_

#include "freertos/FreeRTOS.h"
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

typedef struct {
    String id;
    String name;
    String clientIp;
    String mac;
    String cpu;
    String os;
    int totalmem;
    String manufacturer;
    String modelId;
    String appVersion;
    String appBuild;
} tdDevice;


class Ufo;
class AWSIntegration;
class DynatraceAction;

class DynatraceMonitoring {

public:

    DynatraceMonitoring();
	virtual ~DynatraceMonitoring();
    
    bool Init(Ufo* pUfo, AWSIntegration* pAws);
    void ProcessConfigChange();
    bool Connect();
    bool Run();
    bool Process();
    void Send(String* json);
    void Shutdown();
    
    DynatraceAction* enterAction(String pName);
    DynatraceAction* enterAction(String pName, int pType);
    DynatraceAction* enterAction(String pName, DynatraceAction* pParent);
    DynatraceAction* enterAction(String pName, int pType, DynatraceAction* pParent);
    void leaveAction(DynatraceAction* action);
    void leaveAction(DynatraceAction* action, String* pUrl, ushort pResponseCode, uint pResponseSize);

    void addAction(DynatraceAction* action);
    String getPayload(DynatraceAction* pActions[], __uint8_t pCount);

    __uint32_t getSequence0();
    __uint32_t getSequence1();
    __uint32_t getTimestamp();

    bool mInitialized = false;
    bool mConnected = false;
    bool mActive = false;
    __uint32_t mStartTimestamp;

private:

    DynatraceAction* mAction[100];
    __uint8_t mActionCount;

    __uint32_t seq0;
    __uint32_t seq1;

    Ufo* mpUfo;  
    AWSIntegration* mpAws;
    Config* mpConfig;

    tdDevice mDevice;
    ushort mBatterylevel;
    portMUX_TYPE myMutex;

};

#endif