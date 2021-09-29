#include "SocketBasic.h"

const char* SocketBasic::getHost() const noexcept
{
    return host.get();
}

const char* SocketBasic::getPortString() const noexcept
{
    return portString.get();
}

int SocketBasic::getBufferLength() const noexcept
{
    return buflen;
}