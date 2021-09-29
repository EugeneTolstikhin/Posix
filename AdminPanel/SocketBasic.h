#ifndef __SOCKET__BASIC__H__
#define __SOCKET__BASIC__H__

#include "String.h"

class SocketBasic
{
public:
	SocketBasic() = default;
	virtual ~SocketBasic() = default;

	const char* getHost() const noexcept;
	const char* getPortString() const noexcept;
	int getBufferLength() const noexcept;

private:
	String portString = "12001";
	String host = "127.0.0.1";
	size_t buflen = 256;
};

#endif // __SOCKET__BASIC__H__
