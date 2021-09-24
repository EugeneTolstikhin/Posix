/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#include <signal.h>

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
    
    const char SERCRET_KEY[] = "Give me the points!";

    int port = 0;
    ssize_t lineSize = 0;
    size_t len = 0, buflen = 0;
    char *line = NULL, *host = NULL;

    const char PORT_SERVER[] = "PORT_SERVER: ";
    const char BUFFER_LENGTH[] = "BUFFER_LENGTH: ";
    const size_t MAX_BUFFER_LENGTH = 1024;
    const int POOL_SIZE = 5;

    // Parse config file
    while ((lineSize = getline(&line, &len, cfg)) != -1)
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

    int sockfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    listen(sockfd, POOL_SIZE);

    socklen_t clilen = sizeof(cli_addr);

    int newsockfd;
    char *buffer = malloc(buflen);
    while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) >= 0)
    {
        bzero(buffer, buflen);

        int n = read(newsockfd, buffer, buflen - 1);
        if (n < 0)
        {
            free(buffer);
            close(newsockfd);
            close(sockfd);
            error("ERROR reading from socket");
        }
        else if (strstr(buffer, SERCRET_KEY) != NULL)
        {
            char answer[] = "5";
            send(newsockfd, answer, strlen(answer), 0);
        }

        close(newsockfd);

        if (!keepRunning) break;
    }

    free(buffer);
    close(sockfd);
    
    return 0; 
}