#include "ConfigFile.h"
#include <cstring>
#include <climits>

ConfigFile::ConfigFile(const char* filename, const char* mode)
{
#ifdef __linux__
	file = fopen(filename, mode);
#elif defined _WIN32
	fopen_s(&file, filename, mode);
#endif

	if (file == nullptr)
	{
		throw ("Cannot open file");
	}

	fseek (file, 0, SEEK_END);
	int length = ftell (file);
	fseek (file, 0, SEEK_SET);
	char* buffer = new char[length + 1];
	if (buffer)
	{
		int numBytes = fread (buffer, 1, length, file);
		if (numBytes > 0)
		{
			fileContent = buffer;
		}

		delete[] buffer;
	}
}

ConfigFile::~ConfigFile()
{
	fclose(file);
}

unsigned ConfigFile::readIntParamValue(const char* valueName)
{
	if (valueName == nullptr)
	{
		throw ("Param value is undefined");
	}
	
	const char* begin = strstr(fileContent.get(), valueName);

	if (begin == nullptr)
	{
		throw ("No such parameter in the config file");
	}

	const char* end = strstr(begin, "\n");
	if (end == nullptr)
	{
		end = &fileContent.get()[fileContent.length()];
	}

	unsigned res = 0;
	unsigned lineLen = (end - begin);
	unsigned counter = 0;
	while (counter < lineLen)
	{
		char digit = begin[lineLen - counter - 1];
		++counter;

		if (digit == ' ')
		{
			break;
		}
		else if (digit < '0' || digit > '9')
		{
			throw ("Invalid parameter value");
		}
		else
		{
			res += (digit - '0') * pow(10, counter - 1);
		}
	}

	return res;
}

unsigned ConfigFile::pow(unsigned x, unsigned y)
{
	unsigned res = x;
	if (y == 0)
	{
		return 1;
	}
	else if (y == 1)
	{
		return res;
	}
	
	unsigned p = pow(x, y / 2);
	if (p > USHRT_MAX) // INT part from SQRT(UINT_MAX) = USHRT_MAX
	{
		throw ("Out of range");
	}
	
	if (y % 2 == 0)
	{
		res = p * p;
	}
	else
	{
		res *= p * p;
	}

	return res;
}

String ConfigFile::readStringParamValue(const char* valueName)
{
	if (valueName == nullptr)
	{
		throw ("Param value is undefined");
	}

	const char* begin = strstr(fileContent.get(), valueName);

	if (begin == nullptr)
	{
		throw ("No such parameter in the config file");
	}

	const char* end = strstr(begin, "\n");
	if (end == nullptr)
	{
		end = &fileContent.get()[fileContent.length()];
	}

	unsigned lineLen = (end - begin);
	unsigned counter = 0;
	String res(lineLen - strlen(valueName) - 1);
	while (counter < lineLen)
	{
		char digit = begin[lineLen - counter - 1];
		++counter;

		if (digit == ' ')
		{
			break;
		}
		else
		{
			res[counter - 1] = digit;
		}
	}

	res.reverse();
	return res;
}

