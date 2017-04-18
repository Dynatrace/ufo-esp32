/*
 * ApiStore.h
 *
 *  Created on: 13.04.2017
 *      Author: helmut.spiegl
 */

#ifndef MAIN_APISTORE_H_
#define MAIN_APISTORE_H_

#include "nvs.h"
#include <string>

class ApiStore {
public:
	ApiStore();
	virtual ~ApiStore();

	bool SetApi(__uint8_t uId, std::string& rsApi);
	bool DeleteApi(__uint8_t uId);

	__uint8_t GetApiCount();
	bool GetApi(__uint8_t uId, std::string& rApi);

private:
	bool ReadString(nvs_handle h, const char* sKey, std::string& rsValue);

};

#endif /* MAIN_APISTORE_H_ */
