#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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

#include <errno.h>
#include <limits.h>
#include <stdbool.h>

void error(const char* msg)
{
    perror(msg);
    exit(1);
}

int parseConfigLine(const char* line, const char* param, const long min, const long max)
{
    int res = -1;

    int headerLen = strlen(param);
    int len = strlen(line) - headerLen;

    char* str = (char*)malloc(len + 1);
    if (str == NULL)
    {
        //error("Cannot allocate memory for port");
        return -2;
    }

    memset(str, '\0', len);
    memcpy(str, &line[strlen(param)], len);
    str[len] = '\0';

    errno = 0;
    char* end;
    const long num = strtol(str, &end, 10);
    free(str);

    if (errno == ERANGE)
    {
        //error("Range error occurred");
        return -3;
    }

    if (num <= max && num >= min && min > 0)
    {
        res = (int)num;
    }
    else
    {
        //error("Port is out of range");
        return -4;
    }

    return res;
}

char* parseConfigLineString(const char* line, const char* param)
{
    int headerLen = strlen(param);
    int len = strlen(line) - headerLen - 1;
    char* result = (char*)malloc(len + 1);
    if (result == NULL)
    {
        //fclose(cfg);
        //error("Cannot allocate memory for host");
        return NULL;
    }

    memcpy(result, &line[headerLen], len);
    result[len] = '\0';

    return result;
}

int main(int argc, char* argv[])
{
    //Open config file
    FILE* cfg;
#ifdef __linux__
    cfg = fopen("config.cfg", "r");
#elif defined _WIN32
    fopen_s(&cfg, "config.cfg", "r");
#endif

    if (cfg == NULL)
    {
        error("Cannot open config file");
    }

    char* portno = NULL;

    int lineSize = 0;
    size_t len = 0, buflen = 0;
    char* line = NULL, * host = NULL;

    const char PORT_CLIENT[] = "PORT_CLIENT: ";
    const char HOST[] = "HOST: ";
    const char BUFFER_LENGTH[] = "BUFFER_LENGTH: ";
    const size_t MAX_BUFFER_LENGTH = 1024;

    //Parse config file
#ifdef __linux__
    while ((lineSize = getline(&line, &len, cfg)) != -1)
#elif defined _WIN32
    line = (char*)malloc(101);
    memset(line, '\0', 101);
    while(fgets(line, 100, cfg) != NULL)
#endif
    {
        if (strstr(line, PORT_CLIENT) != NULL)
        {
            portno = parseConfigLineString(line, PORT_CLIENT);
            if (portno == NULL)
            {
                if (host) free(host);
                if (line) free(line);
                fclose(cfg);
                error("Cannot parse port");
            }
        }
        else if (strstr(line, HOST) != NULL)
        {
            host = parseConfigLineString(line, HOST);

            if (host == NULL)
            {
				if (portno) free(portno);
                if (line) free(line);
                fclose(cfg);
                error("Cannot allocate memory for host");
            }
        }
        else if (strstr(line, BUFFER_LENGTH) != NULL)
        {
            int res = parseConfigLine(line, BUFFER_LENGTH, 1, MAX_BUFFER_LENGTH);

            if (res < 0)
            {
				if (portno) free(portno);
                if (host) free(host);
                if (line) free(line);
                fclose(cfg);
                error("Cannot parse buffer length");
            }
            else
            {
                buflen = res;
            }
        }
    }
    fclose(cfg);
    if (line) free(line);

    struct addrinfo hints, *addrs = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (host)
    {
        getaddrinfo(host, portno, &hints, &addrs);
    }

	free(portno);
    free(host);

    int sockfd = 0;
    for (struct addrinfo* addr = addrs; addr != NULL; addr = addr->ai_next)
    {
        sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sockfd < 0)
        {
            //if (host) free(host);
            //error("ERROR opening socket");
        }
        else
        {
            if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) < 0)
            {
				if (addrs) free(addrs);
                error("ERROR connecting");
            }
        }
    }

	if (addrs) free(addrs);

    const char SERCRET_KEY[] = "I wanna be kept in (ms): ";
    const char READY_MESSAGE[] = "Wait for new data!";
    char* buffer = (char*)malloc(buflen);
    if (buffer == NULL)
    {
        error("Cannot allocate memory for buffer");
    }

    memset(buffer, '\0', buflen);
#ifdef __linux__
    int n = read(sockfd, buffer, buflen);
#elif defined _WIN32
    int n = recv(sockfd, buffer, buflen, 0);
#endif
    if (n < 0)
    {
        free(buffer);
        error("ERROR reading ready message from socket");
    }
    else if (strstr(buffer, READY_MESSAGE) != NULL)
    {
        printf("%s\n", buffer);
        char* message = (char*)malloc(buflen * 2);
        if (message == NULL)
        {
            free(buffer);
            error("Cannot allocate memory for message");
        }

#ifdef __linux__
        strcpy(message, SERCRET_KEY);
#elif defined _WIN32
        strcpy_s(message, strlen(SERCRET_KEY), SERCRET_KEY);
#endif

        printf("Please enter the array of ms, how long the server should keep this thread alive: ");

        fgets(buffer, buflen - 1, stdin);

#ifdef __linux__
        strcat(message, buffer);
#elif defined _WIN32
        strcat_s(message, strlen(buffer), buffer);
#endif

        printf("%s", message);

#ifdef __linux__
        n = write(sockfd, message, strlen(message));
#elif defined _WIN32
        n = send(sockfd, message, strlen(message), 0);
#endif
        free(message);

        if (n < 0)
        {
            free(buffer);
            error("ERROR writing to socket");
        }

        memset(buffer, '\0', buflen);
#ifdef __linux__
        n = read(sockfd, buffer, buflen);
#elif defined _WIN32
        int n = recv(sockfd, buffer, buflen, 0);
#endif
        if (n < 0)
        {
            free(buffer);
            error("ERROR reading from socket");
        }

        printf("%s\n", buffer);
    }

#ifdef __linux__
    close(sockfd);
#elif defined _WIN32
    closesocket(sockfd);
#endif

    free(buffer);
    return 0;
}
