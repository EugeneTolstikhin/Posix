#include "SocketClient.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef __linux__

#include <unistd.h>

#elif _WIN32

#include <windows.h>

#endif


#include <cstring>
#include <cstdlib>
#include <cstdio>

void SocketClient::init()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        throw("ERROR opening socket");
    }

	server = gethostbyname(getHost());

    if (server == nullptr)
    {
        throw("ERROR, no such host");
    }

	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(getPort());
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        throw("ERROR connecting");
    }

	int buflen = getBufferLength();
	RAIIArray<char>buffer(buflen);

    int n = read(sockfd, buffer.get(), buflen);
    if (n < 0)
    {
        throw("ERROR reading ready message from socket");
    }
	else if (strstr(buffer.get(), READY_MESSAGE.get()) != nullptr)
    {
        printf("%s\n", buffer.get());
        RAIIArray<char>message(buflen * 2);

        strcpy(message.get(), SECRET_KEY.get());

        printf("Please enter the array of ms, how long the server should keep this thread alive: ");
        
        bzero(buffer.get(), buflen);
        fgets(buffer.get() ,buflen - 1, stdin);
        strcat(message.get(), buffer.get());
        printf("%s", message.get());
        n = write(sockfd, message.get(), strlen(message.get()));
        if (n < 0)
        {
            throw("ERROR writing to socket");
        }

        bzero(buffer.get(), buflen);
        n = read(sockfd, buffer.get(), buflen);
        if (n < 0)
        {
            throw("ERROR reading from socket");
        }

        printf("%s\n", buffer.get());
    }
}

void SocketClient::setSecretKey(const char* key)
{
	int len = strlen(key);
	memcpy(SECRET_KEY.get(), &key[0], len);
    SECRET_KEY[len] = '\0';
}

void SocketClient::setReadyMessage(const char* message)
{
	int len = strlen(message);
	memcpy(READY_MESSAGE.get(), &message[0], len);
    READY_MESSAGE[len] = '\0';
}
