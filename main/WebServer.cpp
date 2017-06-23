#include "WebServer.h"
#include "ufo.h"
#include "config.h"
#include "HttpRequestParser.h"
#include "HttpResponse.h"
#include "DynamicRequestHandler.h"
#include <lwip/sockets.h>
#include <esp_log.h>
#include <esp_system.h>
#include <String.h>

#include "sdkconfig.h"
#include "fontwoff.h"
#include "fontttf.h"
#include "fontsvg.h"
#include "fonteot.h"
#include "indexhtml.h"
#include "keypem.h"
#include "certpem.h"

static char tag[] = "WebServer";

extern const unsigned char certkey_pem_start[] asm("_binary_certkey_pem_start");
extern const unsigned char certkey_pem_end[]   asm("_binary_certkey_pem_end");
unsigned int uWsCertLength = certkey_pem_end - certkey_pem_start;
const unsigned char* sWsCert = certkey_pem_start;

struct TServerSocketPair{
	WebServer* pServer;
	int socket;
	int number;
};

void request_handler_function(void *pvParameter);

//------------------------------------------------------------------

WebServer::WebServer() {
	mpSslCtx = NULL;
	muConcurrentConnections = 0;
	myMutex = portMUX_INITIALIZER_UNLOCKED;
	mbFree = true;
	mbRestart = false;
}

WebServer::~WebServer() {
	SSL_CTX_free(mpSslCtx);
}

__uint8_t WebServer::GetConcurrentConnections(){
	__uint8_t u;
	//taskENTER_CRITICAL(&myMutex);
	u = muConcurrentConnections;
	//taskEXIT_CRITICAL(&myMutex);
	return u;
}

void WebServer::SignalConnection(){
	//taskENTER_CRITICAL(&myMutex);
	muConcurrentConnections++;
	//taskEXIT_CRITICAL(&myMutex);
}

void WebServer::SignalConnectionExit(){
	//taskENTER_CRITICAL(&myMutex);
	if (muConcurrentConnections)
	 	muConcurrentConnections--;
	//taskEXIT_CRITICAL(&myMutex);
}

void WebServer::EnterCriticalSection(){
	while (true){
		taskENTER_CRITICAL(&myMutex);
		if (mbFree){
			mbFree = false;
			taskEXIT_CRITICAL(&myMutex);
			return;
		}
		taskEXIT_CRITICAL(&myMutex);
		vTaskDelay(10);
	}

}

void WebServer::LeaveCriticalSection(){
	taskENTER_CRITICAL(&myMutex);
	mbFree = true;
	taskEXIT_CRITICAL(&myMutex);
}


bool WebServer::Start(){
	__uint16_t port; 
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;

	if (mpUfo->GetConfig().mbAPMode){
		port = 80;
	}
	else{
		if (mpUfo->GetConfig().muWebServerPort)
			port = mpUfo->GetConfig().muWebServerPort;
		else
			port = mpUfo->GetConfig().mbWebServerUseSsl ? 443 : 80;
			
		if (mpUfo->GetConfig().mbWebServerUseSsl){

			if (!mpSslCtx){
				mpSslCtx = SSL_CTX_new(TLS_server_method());
				if (!mpSslCtx) {
					ESP_LOGE(tag, "SSL_CTX_new: %s", strerror(errno));
					return false;
				}
				if (mpUfo->GetConfig().msWebServerCert.length()){
					//ESP_LOGD(tag, "Using custom certificate <%s>", mpUfo->GetConfig().msWebServerCert.c_str());
					sWsCert = (unsigned char*)mpUfo->GetConfig().msWebServerCert.c_str();
					uWsCertLength = mpUfo->GetConfig().msWebServerCert.length();
				}
				if (!SSL_CTX_use_certificate_ASN1(mpSslCtx,  uWsCertLength, sWsCert)){
					ESP_LOGE(tag, "SSL_CTX_use_certificate_ASN1: %s", strerror(errno));
					SSL_CTX_free(mpSslCtx);
					mpSslCtx = NULL;
					return false;
				}
				if (!SSL_CTX_use_PrivateKey_ASN1(0, mpSslCtx, sWsCert,  uWsCertLength)){
					ESP_LOGE(tag, "SSL_CTX_use_PrivateKey_ASN1: %s", strerror(errno));
					SSL_CTX_free(mpSslCtx);
					mpSslCtx = NULL;
					return false;
				}
				port = 443;
			}
		}
	}

	// Create a socket that we will listen upon.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ESP_LOGE(tag, "socket: %d %s", sock, strerror(errno));
		return false;
	}

	// Bind our server socket to a port.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);
	int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc < 0) {
		ESP_LOGE(tag, "bind: %d %s", rc, strerror(errno));
		close(sock);
		return false;
	}

	// Flag the socket as listening for new connections.
	rc = listen(sock, mpUfo->GetConfig().mbWebServerUseSsl ? 1 : 5);
	if (rc < 0) {
		ESP_LOGE(tag, "listen: %d %s", rc, strerror(errno));
		close(sock);
		return false;
	}
	ESP_LOGI(tag, "Webserver started listening on %d %s", port, mpSslCtx ? "(secure)":"");

	int conNumber = 0;
	
	while (1) {

		//while (GetConcurrentConnections() >= 2){
		//	vTaskDelay(100 / portTICK_PERIOD_MS);
		//}
		ESP_LOGD(tag, "--- enter accept - %d connections\n", GetConcurrentConnections()
		);
		// Listen for a new client connection.
		socklen_t clientAddressLength = sizeof(clientAddress);
		int clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (clientSock < 0) {
			ESP_LOGE(tag, "accept: %d %s", clientSock, strerror(errno));
			close(sock);
			return false;
		}
		SignalConnection();
		conNumber++;
		ESP_LOGD(tag, "new connection - %d\n", conNumber);

		/*struct linger li;
        li->l_onoff = 0;
        li->l_linger = 0;
		setsockopt(clientSock, SOL_SOCKET, SO_LINGER, &lin, sizeof(li);
		*/

		if (mpSslCtx)
			WebRequestHandler(clientSock, conNumber);
		else{
			TServerSocketPair* pServerSocketPair = (TServerSocketPair*)malloc(sizeof(TServerSocketPair));
			pServerSocketPair->pServer = this;
			pServerSocketPair->socket = clientSock;
			pServerSocketPair->number = conNumber;
			xTaskCreatePinnedToCore(&request_handler_function, "WebSocketHandler", 12288, pServerSocketPair, 4, NULL, 0);
		}
	}
}

void request_handler_function(void *pvParameter)
{
	TServerSocketPair* serverSocket = (TServerSocketPair*)pvParameter;
	serverSocket->pServer->WebRequestHandler(serverSocket->socket, serverSocket->number);
	delete serverSocket;
	vTaskDelete(NULL);
}

void WebServer::WebRequestHandler(int socket, int conNumber){

	// We now have a new client ...
	int total =	1024;
	char *data = (char*)malloc(total);
	HttpRequestParser httpParser(socket);
	HttpResponse httpResponse;
	DynamicRequestHandler requestHandler(mpUfo, mpDisplayCharterLevel1, mpDisplayCharterLevel2);
	SSL* ssl = NULL;
	bool receivedSomething = false;

	ESP_LOGD(tag, "<%d> WebRequestHandler - heapfree: %d", conNumber, esp_get_free_heap_size());
   
	if (mpSslCtx){
		ESP_LOGD(tag, "<%d> SSL_new", conNumber);
		ssl = SSL_new(mpSslCtx);
		if (!ssl) {
			ESP_LOGE(tag, "<%d> SSL_new: %s", conNumber, strerror(errno));
			goto EXIT;
		}
		ESP_LOGD(tag, "<%d> SSL_new DONE", conNumber);

		SSL_set_fd(ssl, socket);

		if (!WaitForData(socket, 1)){
			ESP_LOGE(tag, "<%d> No Data", conNumber);
			goto EXIT;
		}

		ESP_LOGD(tag, "<%d> Enter SSL_accept", conNumber);
		if (!SSL_accept(ssl)){
			ESP_LOGE(tag, "<%d> SSL_accept %s", conNumber, strerror(errno));
			goto EXIT;
		}
	}
	ESP_LOGD(tag, "<%d> Socket Accepted", conNumber);
	ESP_LOGD(tag, "<%d> WebRequestHandler after - heapfree: %d", conNumber, esp_get_free_heap_size());
   
    while (1){
		httpParser.Init(&mOta);
		//httpParser.AddUploadUrl("/updatecert");

		while(1) {

			ssize_t sizeRead;
			if (ssl)
				sizeRead = SSL_read(ssl, data, total);
			else
				sizeRead = recv(socket, data, total, 0);

			if (sizeRead <= 0) {
				if (receivedSomething){
					ESP_LOGE(tag, "<%d> Connection closed during parsing", conNumber);
				}
				else{
					ESP_LOGD(tag, "<%d> Connection closed during parsing", conNumber);
				}
				goto EXIT;
			}
			ESP_LOGD(tag, "<%d> received %d bytes", conNumber, sizeRead);
			receivedSomething = true;
				
			if (!httpParser.ParseRequest(data, sizeRead)){
				ESP_LOGE(tag, "<%d> HTTP Parsing error: %d", conNumber, httpParser.GetError());
				goto EXIT;
			}
			if (httpParser.RequestFinished()){
				break;
			}
		}

		ESP_LOGI(tag, "<%d> Request parsed: %s", conNumber,  httpParser.GetUrl().c_str());

		if (ssl)
			httpResponse.Init(ssl, httpParser.IsHttp11(), httpParser.IsConnectionClose());
		else
			httpResponse.Init(socket, httpParser.IsHttp11(), httpParser.IsConnectionClose());
		
		if (httpParser.GetUrl().equals("/") || httpParser.GetUrl().equals("/index.html")){
			httpResponse.AddHeader(HttpResponse::HeaderContentTypeHtml);
			httpResponse.AddHeader("Content-Encoding: gzip");
			if (!httpResponse.Send(indexhtml_h, sizeof(indexhtml_h)))
				break;
		}
		else if (httpParser.GetUrl().equals("/fonts/material-design-icons.woff")){
			httpResponse.AddHeader(HttpResponse::HeaderContentTypeBinary);
			if (!httpResponse.Send(fontwoff_h, sizeof(fontwoff_h)))
				break;
		}
		else if (httpParser.GetUrl().equals("/fonts/material-design-icons.ttf")){
			httpResponse.AddHeader(HttpResponse::HeaderContentTypeBinary);
			if (!httpResponse.Send(fontttf_h, sizeof(fontttf_h)))
				break;
		}
		else if (httpParser.GetUrl().equals("/fonts/material-design-icons.eot")){
			httpResponse.AddHeader(HttpResponse::HeaderContentTypeBinary);
			if (!httpResponse.Send(fonteot_h, sizeof(fonteot_h)))
				break;
		}
		else if (httpParser.GetUrl().equals("/fonts/material-design-icons.svg")){
			httpResponse.AddHeader(HttpResponse::HeaderContentTypeBinary);
			if (!httpResponse.Send(fontsvg_h, sizeof(fontsvg_h)))
				break;
		}
		else if (httpParser.GetUrl().equals("/dynatraceintegration")){
			if (!requestHandler.HandleDynatraceIntegrationRequest(httpParser.GetParams(), httpResponse))
				break;
		}
		else if (httpParser.GetUrl().equals("/apilist")){
			if (!requestHandler.HandleApiListRequest(httpParser.GetParams(), httpResponse))
				break;
		}
		else if (httpParser.GetUrl().equals("/apiedit")){
			if (!requestHandler.HandleApiEditRequest(httpParser.GetParams(), httpResponse))
				break;
		}
		else if (httpParser.GetUrl().equals("/info")){
			if (!requestHandler.HandleInfoRequest(httpParser.GetParams(), httpResponse))
				break;
		}
		else if (httpParser.GetUrl().equals("/config")){
			if (!requestHandler.HandleConfigRequest(httpParser.GetParams(), httpResponse))
				break;
		} 
		else if (httpParser.GetUrl().equals("/srvconfig")){
			if (!requestHandler.HandleSrvConfigRequest(httpParser.GetParams(), httpResponse))
				break;
		} 
		else if (httpParser.GetUrl().equals("/firmware")) {
			if (!requestHandler.HandleFirmwareRequest(httpParser.GetParams(), httpResponse))
				break;
		}
		else if (httpParser.GetUrl().equals("/checkfirmware")) {
			if (!requestHandler.HandleCheckFirmwareRequest(httpParser.GetParams(), httpResponse))
				break;
		}
		else if (httpParser.GetUrl().equals("/update")) {
			String sBody = "<html><head><title>SUCCESS - firmware update succeeded, rebooting shortly.</title>"
				           "<meta http-equiv=\"refresh\" content=\"10; url=/\"></head><body>"
						   "<h2>SUCCESS - firmware update succeeded, rebooting shortly.</h2></body></html>";
			if (!httpResponse.Send(sBody))
				break;
		}

		else if (httpParser.GetUrl().equals("/test")){
			String sBody;
			sBody = httpParser.IsGet() ? "GET " : "POST ";
			sBody += httpParser.GetUrl();
			sBody += httpParser.IsHttp11() ? " HTTP/1.1" : "HTTP/1.0";
			sBody += "\r\n";
			std::list<TParam> params = httpParser.GetParams();
			std::list<TParam>::iterator it = params.begin();
			while (it != params.end()){
				sBody += (*it).paramName;
				sBody += " = ";
				sBody += (*it).paramValue;
				sBody += "\r\n";
				it++;
			}
			if (!httpParser.IsGet()){
				sBody += "Boundary:<";
				sBody += httpParser.GetBoundary();
				sBody += ">\r\n";
				sBody += "Body:\r\n";
				sBody += httpParser.GetBody();
			}
			ESP_LOGD(tag, "<%d> test body <%s>", conNumber,  sBody.c_str());
			if (!httpResponse.Send(sBody))
				break;
		}
		else{
			httpResponse.SetRetCode(404);
			if (!httpResponse.Send(NULL, 0))
				break;
		}

		if (mbRestart || requestHandler.ShouldRestart() || (mOta.GetProgress() == OTA_PROGRESS_FINISHEDSUCCESS)){
			ESP_LOGD(tag, "<%d> RESTARTING!", conNumber);
			vTaskDelay(100);
			esp_restart();
		}

		if (httpParser.IsConnectionClose()){
			close(socket);
			break;
		}

		if (!WaitForData(socket, 3)){
			ESP_LOGD(tag, "<%d> No Data", conNumber);
			break;
		}
		else
			ESP_LOGD(tag, "<%d> More data available!", conNumber);
	}

EXIT:
	ESP_LOGD(tag, "<%d> Connection EXIT", conNumber);
	if (ssl)
		SSL_free(ssl);

	free(data);
	close(socket);
	SignalConnectionExit();
}

bool WebServer::WaitForData(int socket, __uint8_t timeoutS){
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = timeoutS;
	return select(FD_SETSIZE, &readfds, 0, 0, &tv);
}
