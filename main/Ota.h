/*
 * Ota.hpp
 *
 */

#ifndef MAIN_OTA_H_
#define MAIN_OTA_H_

#include <esp_ota_ops.h>
//#include <string>
#include "DownAndUploadHandler.h"
#include "String.h"
#include "WebClient.h"

//#define LATEST_FIRMWARE_URL "https://surpro4:9999/getfirmware"  // testing with local go server
#define OTA_LATEST_FIRMWARE_JSON_URL "https://github.com/Dynatrace/ufo-esp32/raw/master/firmware/version.json"
#define OTA_LATEST_FIRMWARE_URL "https://github.com/Dynatrace/ufo-esp32/raw/master/firmware/ufo-esp32.bin"


#define OTA_PROGRESS_NOTYETSTARTED      -1
#define OTA_PROGRESS_CONNECTIONERROR    -2
#define OTA_PROGRESS_FLASHERROR	        -3
#define OTA_PROGRESS_FINISHEDSUCCESS  -200

class Ota : public DownAndUploadHandler {
public:
	static void StartUpdateFirmwareTask();
	//static int  smErrorCode; //TODO this should provide "feedback" from the static class*/

	/*
	*   get firmware update progress in 0..100%
	*   @returns in case of an error, it returns negative error codes
	*/
	static int GetProgress();
	static unsigned int GetTimestamp();

public:
	Ota();
	virtual ~Ota();
	bool UpdateFirmware(String url);
	

	bool SwitchBootPartition();

	bool InternalOnRecvBegin(bool isContentLength, unsigned int contentLength);

public:
	bool OnReceiveBegin(unsigned short int httpStatusCode, bool isContentLength, unsigned int contentLength);
	bool OnReceiveBegin(String& sUrl, unsigned int contentLength);
	bool OnReceiveEnd();
	bool OnReceiveData(char* buf, int len); // override DownloadHandler virtual method



private:
	WebClient mWebClient;
    esp_ota_handle_t mOtaHandle = 0 ;
    const esp_partition_t *mpUpdatePartition = NULL;
    unsigned int muActualDataLength = 0;
	unsigned int muContentLength = 0;
	static volatile int miProgress; 
	static volatile unsigned int muTimestamp;
};

#endif /* MAIN_OTA_H_ */
