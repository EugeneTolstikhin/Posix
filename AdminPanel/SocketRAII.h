#ifndef __SOCKET_RAII_H__
#define __SOCKET_RAII_H__

class SocketRAII
{
public:
	SocketRAII(int af, int type, int protocol);
	~SocketRAII();

	int get() const noexcept;
	void set(int af, int type, int protocol) noexcept;
	bool available() const noexcept;
private:
	int sockfd = -1;
	bool isAvailable = false;
};

#endif // __SOCKET_RAII_H__
