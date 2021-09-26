#ifndef __FILE__RAII__HPP__
#define __FILE__RAII__HPP__

#include <cstdio>
#include <cstdlib>

class FileRAII
{
public:
	FileRAII(const char* filename, const char* mode)
	{
		m_resource = fopen(filename, mode);

		if (m_resource == nullptr)
		{
			throw("Cannot open file");
		}
	}

	char* getLine()
	{
		size_t lineLen = 0;
		if (getline(&m_line, &lineLen, m_resource) < 0)
		{
			return nullptr;
		}

		return m_line;
	}

	~FileRAII()
	{
		if (m_line) free(m_line);
		fclose(m_resource);
	}
	
private:
	FILE* m_resource;
	char* m_line;
};

#endif // __FILE__RAII__HPP__
