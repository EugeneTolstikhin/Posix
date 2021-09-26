#ifndef __SOCKET__CLIENT__H__
#define __SOCKET__CLIENT__H__

#include "SocketBasic.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "RAIIArray.hpp"

class SocketClient: private SocketBasic
{
public:
    SocketClient() = default;
    virtual ~SocketClient() = default;

	void init();
	void setSecretKey(const char*);
	void setReadyMessage(const char*);
private:
	struct sockaddr_in serv_addr;
    struct hostent *server;
	int sockfd;

	RAIIArray<char> SECRET_KEY;
    RAIIArray<char> READY_MESSAGE;
};

#endif // __SOCKET__CLIENT__H__
