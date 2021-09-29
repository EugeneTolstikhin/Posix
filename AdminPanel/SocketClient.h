#ifndef __SOCKET__CLIENT__H__
#define __SOCKET__CLIENT__H__

#include "SocketBasic.h"
#include "String.h"
#include "SocketRAII.h"

#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#elif defined _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#endif

class SocketClient : private SocketBasic
{
public:
	SocketClient();
	virtual ~SocketClient() = default;

	void run();
private:
};

#endif // __SOCKET__CLIENT__H__
