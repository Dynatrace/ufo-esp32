#ifndef MAIN_AWSINTEGRATION_H_
#define MAIN_AWSINTEGRATION_H_

#include <aws_iot_mqtt_client_interface.h>
#include "Config.h"
#include "String.h"

class Ufo;
class AWSIntegration {

public:

    AWSIntegration();
	virtual ~AWSIntegration();
    
    bool Init(Ufo* pUfo);
    bool Connect();
    void Shutdown();
    bool Run();
    bool Publish(const char* pTopic, short pTopicLenth, String* pPayload);

    bool mInitialized = false;
    bool mConnected = false;
    bool mActive = false;

private:

    Ufo* mpUfo;  

    AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams;
	IoT_Client_Connect_Params connectParams;
};

#endif