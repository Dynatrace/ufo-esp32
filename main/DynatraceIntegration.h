#include "WebClient.h"
#include "Url.h"
#include "Ufo.h"
#include "DisplayCharter.h"
#include "Config.h"
#include "String.h"


class DynatraceIntegration {

public:

    DynatraceIntegration(Ufo* pUfo, DisplayCharter* pDisplayLowerRing, DisplayCharter* pDisplayUpperRing);
	virtual ~DynatraceIntegration();
    
    bool Init();
    void Shutdown();
    void Poll();

    bool mInitialized;
    bool mActive;

private:
    WebClient  dtClient;

    Ufo* mpUfo;  
    DisplayCharter* mpDisplayLowerRing;
    DisplayCharter* mpDisplayUpperRing;
    Config* mpConfig;
    
    Url mpDtUrl;

    String mDtEnvId;
    String mDtApiToken;
    String mUrl;

    int miApplicationProblems;
    int miServiceProblems;
    int miInfrastructureProblems;
};

