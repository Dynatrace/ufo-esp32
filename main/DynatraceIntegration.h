#include "WebClient.h"
#include "Url.h"
#include "DownAndUploadHandler.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "String.h"
#include <cJSON.h>


class DynatraceIntegration : public DownAndUploadHandler {

public:

    DynatraceIntegration(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing);
	virtual ~DynatraceIntegration();
    
    bool Init();
    void Shutdown();
    bool Poll();
    bool GetData();

    bool OnReceiveBegin(String& sUrl, unsigned int contentLength);
    bool OnReceiveBegin(unsigned short int httpStatusCode, bool isContentLength, unsigned int contentLength);
    bool OnReceiveData(char* buf, int len);
    bool OnReceiveEnd();

    bool mInitialized;
    bool mActive;

private:

    bool Process();
    void DisplayDefault();
    void HandleFailure();

    WebClient  dtClient;

    Ufo* mpUfo;  
    DisplayCharter* mpDisplayLowerRing;
    DisplayCharter* mpDisplayUpperRing;
    Config* mpConfig;
    
    Url mpDtUrl;

    String mDtEnvId;
    String mDtApiToken;
    String mUrl;
    String mJson;

    cJSON* json;

    int miTotalProblems;
    int miApplicationProblems;
    int miServiceProblems;
    int miInfrastructureProblems;
};

