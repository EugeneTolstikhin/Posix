#ifndef __SOCKET__BASIC__H__
#define __SOCKET__BASIC__H__

#include <cstdlib>
#include "String.h"

class SocketBasic
{
public:
	SocketBasic();
	virtual ~SocketBasic() = default;

	const char* getHost() const noexcept;
	const char* getPortString() const noexcept;
	size_t getBufferLength() const noexcept;
private:
	String portString;
	String host;
	size_t buflen;
};

#endif // __SOCKET__BASIC__H__
