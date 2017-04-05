/*
 * Server.h
 *
 *  Created on: 31.03.2017
 *      Author: helmut.spiegl
 */

#ifndef MAIN_SERVER_H_
#define MAIN_SERVER_H_


#include "freertos/FreeRTOS.h"

class Server {
public:
	Server();
	virtual ~Server();

	bool Start(__uint16_t port);

	void RequestHandler(int socket);

private:

};

#endif /* MAIN_SERVER_H_ */
