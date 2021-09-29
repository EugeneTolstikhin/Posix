#include "SocketClient.h"

#include <sys/types.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>

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

SocketClient::SocketClient() : SocketBasic()
{
	//
}

void SocketClient::run()
{
    struct addrinfo hints, *addrs = nullptr;
    memset(&hints, 0, (sizeof hints));
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	int n = getaddrinfo(getHost(), getPortString(), &hints, &addrs);
	if (n != 0)
    {
        throw("Cannot get address information");
    }
	
	if (addrs == nullptr)
	{
		throw("Address info data is empty");
	}

    SocketRAII s(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    while (!s.available())
    {
        if (addrs != nullptr && addrs->ai_next != nullptr)
        {
			addrs = addrs->ai_next;
            s.set(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        }
        else
        {
            if (addrs) free(addrs);
        	throw("ERROR no proper socket found");
        }
    };

	n = connect(s.get(), addrs->ai_addr, addrs->ai_addrlen);
    if (n != 0)
    {
        if (addrs) free(addrs);
        throw("ERROR connecting");
    }

    if (addrs) free(addrs);

    const char SECRET_KEY[] = "I wanna be kept in (ms): ";
    const char READY_MESSAGE[] = "Wait for new data!";

	char* buf = (char*)malloc(getBufferLength());
    n = recv(s.get(), buf, getBufferLength(), 0);
	String buffer(buf);
	free(buf);

    if (n < 0)
    {
        throw("ERROR reading ready message from socket");
    }
	else if (n == 0)
	{
		throw("ERROR socket may be shutdown");
	}
    else if (buffer.found(String(READY_MESSAGE)))
    {
        String message(SECRET_KEY);

        printf("Please enter the array of ms, how long the server should keep this thread alive: ");

		char* buf = (char*)malloc(getBufferLength());
        fgets(buf, getBufferLength(), stdin);
        message.concat(String(buf));

        n = send(s.get(), message.get(), message.length(), 0);
        if (n < 0)
        {
			free(buf);
            throw("ERROR writing to socket");
        }

        memset(buf, '\0', getBufferLength());
        n = recv(s.get(), buf, getBufferLength(), 0);
		printf("%s\n", buf);
		free(buf);
        if (n < 0)
        {
            throw("ERROR reading from socket");
        }
    }
}
