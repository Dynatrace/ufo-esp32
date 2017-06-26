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
    
    bool Init();
    bool Init(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing);
    bool Connect();
    void Shutdown();
    bool Run();
    bool Poll();
    bool GetData();

    bool OnReceiveBegin(String& sUrl, unsigned int contentLength);
    bool OnReceiveBegin(unsigned short int httpStatusCode, bool isContentLength, unsigned int contentLength);
    bool OnReceiveData(char* buf, int len);
    bool OnReceiveEnd();

    bool mInitialized = false;
    bool mActive = false;

    String& getEnvId() { return mDtEnvId; }

private:

    bool Process(String& jsonString);
    void DisplayDefault();
    void HandleFailure();

    WebClient  dtClient;

    Ufo* mpUfo;  
    DisplayCharter* mpDisplayLowerRing;
    DisplayCharter* mpDisplayUpperRing;
    Config* mpConfig;
//	Wifi* mpWifi;
    
    Url mDtUrl;

    String mDtEnvId;
    String mDtApiToken;
    String mUrl;

    int miTotalProblems;
    int miApplicationProblems;
    int miServiceProblems;
    int miInfrastructureProblems;
};

#endif