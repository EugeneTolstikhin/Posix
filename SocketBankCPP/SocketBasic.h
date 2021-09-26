#ifndef __SOCKET__BASIC__H__
#define __SOCKET__BASIC__H__

#include <cstdlib>
#include "RAIIArray.hpp"

class SocketBasic
{
public:
    SocketBasic() = default;
    ~SocketBasic() = default;
	void parseConfigFile(const char*);

	const char* getHost() const noexcept;
	int getPort() const noexcept;
	int getBufferLength() const noexcept;
	
private:
	int parseConfigLine(const char* line, const char* param, long min, long max);
	RAIIArray<char> parseConfigLineString(const char* line, const char* param);
	
	const char *PORT_CLIENT = "PORT_CLIENT: ";
    const char *HOST = "HOST: ";
    const char *BUFFER_LENGTH = "BUFFER_LENGTH: ";
    const size_t MAX_BUFFER_LENGTH = 1024;

	int port = 0;
	size_t buflen = 0;
	RAIIArray<char> host;
};

#endif // __SOCKET__BASIC__H__
