#include "SocketBasic.h"
#include "FileRAII.hpp"
#include "RAII.hpp"

#include <cstring>
#include <climits>
#include <cstdlib>
#include <cerrno>

void SocketBasic::parseConfigFile(const char* filename)
{
    FileRAII cfg(filename, "r");
	
	RAII<char> line;
	while ((line = cfg.getLine()) != nullptr)
    {
        if (strstr(line.get(), PORT_CLIENT) != nullptr)
        {
            port = parseConfigLine(line.get(), PORT_CLIENT, 1, USHRT_MAX);

            if (port < 0)
            {
                throw("Cannot parse port");
            }
        }
        else if (strstr(line.get(), HOST) != nullptr)
        {
            host = parseConfigLineString(line.get(), HOST);

            if (host.get() == nullptr)
            {
                throw("Cannot allocate memory for host");
            }
        }
        else if (strstr(line.get(), BUFFER_LENGTH) != nullptr)
        {
            int res = parseConfigLine(line.get(), BUFFER_LENGTH, 1, MAX_BUFFER_LENGTH);

            if (res < 0)
            {
                throw("Cannot parse buffer length");
            }
            else
            {
                buflen = res;
            }
        }
    }
}

int SocketBasic::parseConfigLine(const char* line, const char* param, long min, long max)
{
    int res = -1;
    
    int headerLen = strlen(param);
    int len = strlen(line) - headerLen;

    RAIIArray<char> str(len);
    memcpy(str.get(), &line[strlen(param)], len);
    str[len] = '\0';

	errno = 0;
    char* end;
    const long num = strtol(str.get(), &end, 10);

    if (errno == ERANGE)
    {
        throw("Range error occurred");
    }

    if (num <= max && num >= min && min > 0)
    {
        res = static_cast<int>(num);
    }
    else
    {
        throw("Value is out of range");
    }

    return res;
}

RAIIArray<char> SocketBasic::parseConfigLineString(const char* line, const char* param)
{
    int headerLen = strlen(param);
    int len = strlen(line) - headerLen - 1;
    RAIIArray<char> result(len);
    memcpy(result.get(), &line[headerLen], len);
    result[len] = '\0';
    return result;
}

const char* SocketBasic::getHost() const noexcept
{
	return host.get();
}

int SocketBasic::getPort() const noexcept
{
	return port;
}

int SocketBasic::getBufferLength() const noexcept
{
	return buflen;
}