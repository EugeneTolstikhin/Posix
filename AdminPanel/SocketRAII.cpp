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

SocketRAII::SocketRAII(int af, int type, int protocol)
{
	set(af, type, protocol);
}

int SocketRAII::get() const noexcept
{
	return sockfd;
}

void SocketRAII::set(int af, int type, int protocol) noexcept
{
	sockfd = socket(af, type, protocol);
	if (sockfd > 0)
	{
		isAvailable = true;
	}
}

bool SocketRAII::available() const noexcept
{
	return isAvailable;
}

SocketRAII::~SocketRAII()
{
#ifdef __linux__
	close(sockfd);
#elif defined _WIN32
	closesocket(sockfd);
#endif
}