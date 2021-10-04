#ifndef __CONFIG__FILE__H__
#define __CONFIG__FILE__H__

#include <cstdio>
#include "String.h"

class ConfigFile
{
public:
	ConfigFile(const char* filename, const char* mode);
	~ConfigFile();
	unsigned readIntParamValue(const char* valueName);
	String readStringParamValue(const char* valueName);
	
private:
	unsigned pow(unsigned, unsigned);
	
	FILE* file = nullptr;
	String fileContent;
};

#endif // __CONFIG__FILE__H__
