//Server.c

/* The port number is passed as an argument */
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

#include <signal.h>
#include <time.h>

static bool keepRunning = true;

static void intHandler(int dummy)
{
    keepRunning = false;
}

void error(const char* msg)
{
    perror(msg);
    exit(1);
}

int parseConfigLine(const char* line, const char* param, long min, long max)
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

int main(int argc, char* argv[])
{
#ifdef __linux__
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, NULL);
#elif defined _WIN32
    signal(SIGINT, intHandler);
#endif

    srand(time(NULL));

    // Open config file
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

    int port = 0;
    int lineSize = 0;
    size_t len = 0, buflen = 0;
    char* line = NULL, * host = NULL;

    const char PORT_SERVER[] = "PORT_SERVER: ";
    const char BUFFER_LENGTH[] = "BUFFER_LENGTH: ";
    const size_t MAX_BUFFER_LENGTH = 1024;
    const int POOL_SIZE = 5;
    const int RAND_MAXIMUM = 11;
	const size_t MAX_LINE_LENGTH = 100;

    // Parse config file
    line = (char*)malloc(MAX_LINE_LENGTH);
    memset(line, '\0', MAX_LINE_LENGTH);
    while (fgets(line, MAX_LINE_LENGTH - 1, cfg) != NULL)
    {
        if (strstr(line, PORT_SERVER) != NULL)
        {
            port = parseConfigLine(line, PORT_SERVER, 1, USHRT_MAX);

            if (port < 0)
            {
                if (line) free(line);
                fclose(cfg);
                error("Cannot parse port");
            }
        }
        else if (strstr(line, BUFFER_LENGTH) != NULL)
        {
            int res = parseConfigLine(line, BUFFER_LENGTH, 1, MAX_BUFFER_LENGTH);

            if (res < 0)
            {
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

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    struct sockaddr_in serv_addr, cli_addr;
    memset((char*)&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    if (listen(sockfd, POOL_SIZE) < 0)
    {
#ifdef __linux__
        close(sockfd);
#elif defined _WIN32
        closesocket(sockfd);
#endif

        error("SERVER ERROR on listening");
    }

    socklen_t clilen = sizeof(cli_addr);

    const char SERCRET_KEY[] = "Give me the points!";
    const char READY_MESSAGE[] = "Wait for command!";

    int newsockfd;
    char* buffer = (char*)malloc(buflen);
    while ((newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen)) >= 0)
    {
        send(newsockfd, READY_MESSAGE, strlen(READY_MESSAGE), 0);
        memset(buffer, '\0', buflen);

    	int n = recv(newsockfd, buffer, buflen - 1, 0);

        if (n < 0)
        {
            free(buffer);

            #ifdef __linux__
                close(newsockfd);
                close(sockfd);
            #elif defined _WIN32
                closesocket(newsockfd);
                closesocket(sockfd);
            #endif

            error("ERROR reading from socket");
        }
        else if (strstr(buffer, SERCRET_KEY) != NULL)
        {
            int r = rand() % RAND_MAXIMUM;
            char answer[2];

#ifdef __linux__
            sprintf(answer, "%d", r);
#elif defined _WIN32
            sprintf_s(answer, "%d", r);
#endif

            send(newsockfd, answer, strlen(answer), 0);
        }

        #ifdef __linux__
            close(newsockfd);
        #elif defined _WIN32
            closesocket(newsockfd);
        #endif

        if (!keepRunning) break;
    }

    free(buffer);
#ifdef __linux__
    close(sockfd);
#elif defined _WIN32
    closesocket(sockfd);
#endif

    return 0;
}
