/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#include <signal.h>

#include <time.h>

struct list
{
    int value;
    struct list* next;
};

static bool keepRunning = true;

static void intHandler(int dummy)
{
    keepRunning = false;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void runClient(char** host, int port, int buflen, long* points)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        if (*host) free(*host);
        error("CLIENT ERROR opening socket");
    }

    if (*host)
    {
        server = gethostbyname(*host);
    }

    if (server == NULL)
    {
        if (*host) free(*host);
        error("CLIENT ERROR, no such host");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        if (*host) free(*host);
        error("CLIENT ERROR connecting");
    }

    const char SERCRET_KEY[] = "Give me the points!";
    const char READY_MESSAGE[] = "Wait for command!";

    char *buffer = malloc(buflen);
    if (buffer == NULL)
    {
        if (*host) free(*host);
        error("CLIENT Cannot allocate memory for buffer");
    }

    int n = read(sockfd, buffer, buflen);
    if (n < 0)
    {
        free(buffer);
        if (*host) free(*host);
        error("CLIENT ERROR reading from socket");
    }
    else if (strstr(buffer, READY_MESSAGE) != NULL)
    {
        n = write(sockfd, SERCRET_KEY, strlen(SERCRET_KEY));
        if (n < 0)
        {
            free(buffer);
            if (*host) free(*host);
            error("CLIENT ERROR writing to socket");
        }

        bzero(buffer, buflen);
        n = read(sockfd, buffer, buflen);
        if (n < 0)
        {
            free(buffer);
            if (*host) free(*host);
            error("CLIENT ERROR reading from socket");
        }

        errno = 0;
        char *end;
        const long num = strtol(buffer, &end, 10);
        free(buffer);

        if (errno == ERANGE)
        {
            if (*host) free(*host);
            error("CLIENT Range error occurred");
        }

        if (num >= 0 && num < LONG_MAX && (*points + num < LONG_MAX))
        {
            *points += num;
        }
        else
        {
            if (*host) free(*host);
            error("CLIENT Port is out of range");
        }
    }

    close(sockfd);
}

long collectPoints(int timeout, char** host, int port, int buflen)
{
    long points = 0;
    int msec = 0;    
    clock_t before = clock();
    if (before == -1)
    {
        return -1;
    }

    do
    {
        runClient(host, port, buflen, &points);

        clock_t difference = clock() - before;
        if (difference < 0)
        {
            return -1;
        }

        msec = difference * 1000 / CLOCKS_PER_SEC;
    } while ( msec < timeout );

    printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
    fflush(stdout);

    return points;
}

int parseConfigLine(const char* line, const char* param, long min, long max)
{
    int res = -1;
    
    int headerLen = strlen(param);
    int len = strlen(line) - headerLen;

    char *str = malloc(len + 1);
    if (str == NULL)
    {
        //error("Cannot allocate memory for port");
        return -2;
    }

    bzero(str, len);
    memcpy(str, &line[strlen(param)], len);
    str[len] = '\0';

    errno = 0;
    char *end;
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
    char* result = malloc(len + 1);
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

void freeList(struct list* start)
{
    struct list* nodeToDelete = start;
    do
    {
        start = start->next;
        free(nodeToDelete);
        nodeToDelete = start;
    } while (start != NULL);
}

struct list* initNode()
{
    struct list* node = malloc(sizeof(struct list));
    node->value = 0;
    node->next = NULL;
    return node;
}

void runServer(int server_port, int buflen, char** host, int client_port)
{
    const int POOL_SIZE = 5;

    // Open socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("SERVER ERROR opening socket");
    }

    const char SERCRET_KEY[] = "I wanna be kept in (ms): ";
    const char READY_MESSAGE[] = "Wait for new data!";
    const char ACCEPT[] = "Accepted";
    const char REJECT[] = "Rejected";

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(server_port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sockfd);
        error("SERVER ERROR on binding");
    }

    if (listen(sockfd, POOL_SIZE) < 0)
    {
        close(sockfd);
        error("SERVER ERROR on listening");
    }

    socklen_t clilen = sizeof(cli_addr);

    int newsockfd;
    struct list* start = NULL;
    char *buffer = malloc(buflen);

    while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) >= 0)
    {
        send(newsockfd, READY_MESSAGE, strlen(READY_MESSAGE), 0);
        bzero(buffer, buflen);

        int n = read(newsockfd, buffer, buflen - 1);
        if (n < 0)
        {
            free(buffer);
            close(newsockfd);
            close(sockfd);
            error("SERVER ERROR reading from socket");
        }
        else if (strstr(buffer, SERCRET_KEY) != NULL)
        {
            int dataLen = strlen(buffer) - strlen(SERCRET_KEY);

            char* data = malloc(dataLen + 1);
            if (data == NULL)
            {
                continue;
            }
            
            bzero(data, dataLen);
            memcpy(data, &buffer[strlen(SERCRET_KEY)], dataLen);
            data[dataLen] = '\0';

            struct list* chain = initNode();
            start = chain;

            bool stopTokenizing = false;
            char* token = strtok(data, " ");
            while (token != NULL)
            {
                errno = 0;
                char *end;
                const long num = strtol(token, &end, 10);

                if (errno == ERANGE)
                {
                    stopTokenizing = true;
                    break;
                }

                if (num <= USHRT_MAX && num > 0)
                {
                    chain->value = num;
                    chain->next = initNode();
                    chain = chain->next;
                }
                else
                {
                    stopTokenizing = true;
                    break;
                }
                token = strtok(NULL, " ");
            }

            free(data);

            if (stopTokenizing)
            {
                send(newsockfd, REJECT, strlen(REJECT), 0);
            }
            else
            {
                send(newsockfd, ACCEPT, strlen(ACCEPT), 0);

                struct list* node = start;
                do
                {
                    if (node->value > 0)
                    {
                        long retCode = collectPoints(node->value, host, client_port, buflen);
                        if (retCode < 0)
                        {
                            freeList(start);
                            free(buffer);
                            close(newsockfd);
                            close(sockfd);

                            error("SERVER Problems with timer");
                        }
                        else
                        {
                            printf("Total amount of points = %ld\n", retCode);
                            fflush(stdout);
                        }
                    }

                    node = node->next;
                } while (node != NULL);
            }
            
            freeList(start);
        }

        close(newsockfd);

        if (!keepRunning) break;
    }

    free(buffer);
    close(sockfd);
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, NULL);

    // Open config file
    FILE* cfg = fopen("config.cfg", "r");
    if (cfg == NULL)
    {
        error("Cannot open config file");
    }

    int port_client = 0, port_server = 0;
    ssize_t lineSize = 0;
    size_t len = 0, buflen = 0;
    char *line = NULL, *host = NULL;

    const char PORT_CLIENT[] = "PORT_CLIENT: ";
    const char PORT_SERVER[] = "PORT_SERVER: ";
    const char HOST[] = "HOST: ";
    const char BUFFER_LENGTH[] = "BUFFER_LENGTH: ";
    const size_t MAX_BUFFER_LENGTH = 1024;

    // Parse config file
    while ((lineSize = getline(&line, &len, cfg)) != -1)
    {
        if (strstr(line, PORT_CLIENT) != NULL)
        {
            port_client = parseConfigLine(line, PORT_CLIENT, 1, USHRT_MAX);

            if (port_client < 0)
            {
                if (host) free(host);
                if (line) free(line);
                fclose(cfg);
                error("Cannot parse port");
            }
        }
        if (strstr(line, PORT_SERVER) != NULL)
        {
            port_server = parseConfigLine(line, PORT_SERVER, 1, USHRT_MAX);

            if (port_server < 0)
            {
                if (host) free(host);
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
        else if (strstr(line, HOST) != NULL)
        {
            host = parseConfigLineString(line, HOST);

            if (host == NULL)
            {
                if (line) free(line);
                fclose(cfg);
                error("Cannot allocate memory for host");
            }
        }
    }
    fclose(cfg);
    if (line) free(line);

    runServer(port_client, buflen, &host, port_server);

    if (host) free(host);

    return 0; 
}