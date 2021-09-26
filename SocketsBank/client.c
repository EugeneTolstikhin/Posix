#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <errno.h>
#include <limits.h>
#include <stdbool.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int parseConfigLine(const char* line, const char* param, const long min, const long max)
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

int main(int argc, char *argv[])
{
    //Open config file
    FILE* cfg = fopen("config.cfg", "r");
    if (cfg == NULL)
    {
        error("Cannot open config file");
    }
    
    int portno = 0;
    ssize_t lineSize = 0;
    size_t len = 0, buflen = 0;
    char *line = NULL, *host = NULL;

    const char PORT_CLIENT[] = "PORT_CLIENT: ";
    const char HOST[] = "HOST: ";
    const char BUFFER_LENGTH[] = "BUFFER_LENGTH: ";
    const size_t MAX_BUFFER_LENGTH = 1024;
 
    //Parse config file
    while ((lineSize = getline(&line, &len, cfg)) != -1)
    {
        if (strstr(line, PORT_CLIENT) != NULL)
        {
            portno = parseConfigLine(line, PORT_CLIENT, 1, USHRT_MAX);

            if (portno < 0)
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

    struct sockaddr_in serv_addr;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        if (host) free(host);
        error("ERROR opening socket");
    }

    if (host)
    {
        server = gethostbyname(host);
        free(host);
    }

    if (server == NULL)
    {
        error("ERROR, no such host");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }

    const char SERCRET_KEY[] = "I wanna be kept in (ms): ";
    const char READY_MESSAGE[] = "Wait for new data!";
    char *buffer = malloc(buflen);
    if (buffer == NULL)
    {
        error("Cannot allocate memory for buffer");
    }

    bzero(buffer, buflen);
    int n = read(sockfd, buffer, buflen);
    if (n < 0)
    {
        free(buffer);
        error("ERROR reading ready message from socket");
    }
    else if (strstr(buffer, READY_MESSAGE) != NULL)
    {
        printf("%s\n", buffer);
        char *message = malloc(buflen * 2);
        if (message == NULL)
        {
            free(buffer);
            error("Cannot allocate memory for message");
        }

        strcpy(message, SERCRET_KEY);

        printf("Please enter the array of ms, how long the server should keep this thread alive: ");
        
        bzero(buffer,buflen);
        fgets(buffer,buflen - 1,stdin);
        strcat(message, buffer);
        printf("%s", message);
        n = write(sockfd, message, strlen(message));
        free(message);

        if (n < 0)
        {
            free(buffer);
            error("ERROR writing to socket");
        }

        bzero(buffer, buflen);
        n = read(sockfd, buffer, buflen);
        if (n < 0)
        {
            free(buffer);
            error("ERROR reading from socket");
        }

        printf("%s\n", buffer);
    }

    close(sockfd);
    free(buffer);
    return 0;
}