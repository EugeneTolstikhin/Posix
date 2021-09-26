#ifndef __SOCKET__SERVER__H__
#define __SOCKET__SERVER__H__

#include "SocketBasic.h"
class SocketServer: private SocketBasic
{
public:
    SocketServer() = default;
    virtual ~SocketServer() = default;
private:
};

#endif // __SOCKET__SERVER__H__
