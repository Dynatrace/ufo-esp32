#ifndef MAIN_DYNATRACEINTEGRATION_H_
#define MAIN_DYNATRACEINTEGRATION_H_

#include "WebClient.h"
#include "Url.h"
#include "Wifi.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "String.h"
#include <cJSON.h>


class Ufo;

class DynatraceIntegration {

public:

    DynatraceIntegration();
	virtual ~DynatraceIntegration();
    
    void Init(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing);
    void ProcessConfigChange();
    void Run(__uint8_t uTaskId);

private:

    void GetData();
    void Process(String& jsonString);
    void DisplayDefault();
    void HandleFailure();

    WebClient  dtClient;

    Ufo* mpUfo;  
    DisplayCharter* mpDisplayLowerRing;
    DisplayCharter* mpDisplayUpperRing;
    Config* mpConfig;
//	Wifi* mpWifi;
    
    Url mDtUrl;
    String mDtUrlString;
    
    bool mInitialized = false;
    bool mEnabled;
    __uint8_t mActTaskId; 
    __uint8_t mActConfigRevision;

    int miTotalProblems;
    int miApplicationProblems;
    int miServiceProblems;
    int miInfrastructureProblems;
};

#endif