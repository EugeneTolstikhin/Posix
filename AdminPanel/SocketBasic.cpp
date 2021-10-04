#include "SocketBasic.h"
#include "ConfigFile.h"

SocketBasic::SocketBasic()
{
	ConfigFile cfg("config.cfg", "r");
	buflen = cfg.readIntParamValue("BUFFER_LENGTH");
	portString = cfg.readStringParamValue("PORT_CLIENT");
	host = cfg.readStringParamValue("HOST");
}

const char* SocketBasic::getHost() const noexcept
{
    return host.get();
}

const char* SocketBasic::getPortString() const noexcept
{
    return portString.get();
}

size_t SocketBasic::getBufferLength() const noexcept
{
    return buflen;
}