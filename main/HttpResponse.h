#ifndef MAIN_HTTPRESPONSE_H_
#define MAIN_HTTPRESPONSE_H_

#include <string>
#include <list>
#include <lwip/sockets.h>


class HttpResponse {
public:
	HttpResponse() {};
	virtual ~HttpResponse() {};

	void Init(int socket, bool bHttp11, bool bConnectionClose);
	void Init(int socket, __uint16_t uRetCode, bool bHttp11, bool bConnectionClose);
	void SetRetCode(__uint16_t uRetCode) { muRetCode = uRetCode; };
	void AddHeader(const char* sHeader);
	void AddHeader(const char* sName, __uint16_t  uValue);

	bool Send(const char* sBody, __uint16_t uBodyLen);
	bool Send() { return Send(NULL, 0); };

private:
	bool SendInternal(int socket, const char* sData, __uint16_t uLen) { return send(socket, sData, uLen, 0) == uLen; };
	const char* GetResponseMsg(__uint16_t uRetCode, __uint8_t& ruLen);
	__uint8_t Number2String(__uint16_t uNum, char* sBuf);

private:
	int mSocket;
	__uint16_t muRetCode;
	bool mbHttp11;
	bool mbConnectionClose;
	std::list<std::string> mHeaders;

};

#endif
