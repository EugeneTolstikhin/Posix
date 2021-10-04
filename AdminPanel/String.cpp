#include "String.h"
#include <cstring>
#include <cstdio>

String::String() :
	m_size(0)
	,m_resource(nullptr)
{
	//
}

String::String(size_t len) :
	m_size(len)
	,m_resource(new char[len])
{
	//
}

String::String(const char* other)
{
	if (other == nullptr)
	{
		m_size = 0;
		m_resource = nullptr;
	}
	else
	{
		m_size = strlen(other);
		m_resource = new char[m_size];
		newstrcpy(&m_resource, other);
	}
}

String::String(const String& other) :
	m_size(other.m_size)
	, m_resource(new char[other.m_size])
{
	newstrcpy(&m_resource, other.get());
}

String::~String()
{
	if (m_resource != nullptr)
	{
		delete[] m_resource;
		m_resource = nullptr;
	}
}

String& String::operator = (const String& other)
{
	m_size = other.m_size;
	m_resource = new char[other.m_size];
	newstrcpy(&m_resource, other.get());
	return *this;
}

String& String::operator = (const char* other)
{
	if (other == nullptr)
	{
		m_size = 0;
		m_resource = nullptr;
	}
	else
	{
		m_size = strlen(other);
		m_resource = new char[m_size];
		newstrcpy(&m_resource, other);
	}
	
	return *this;
}

char& String::operator [] (size_t idx)
{
	if (idx <= m_size && m_size > 0)
	{
		return m_resource[idx];
	}
	else
	{
		throw("Index out of range");
	}
}

bool String::operator != (const String& other) const noexcept
{
	if (m_resource != nullptr && other.m_resource != nullptr)
	{
		return strcmp(m_resource, other.m_resource) != 0;
	}
	else
	{
		return false;
	}
}

bool String::operator == (const String& other) const noexcept
{
	if (m_resource != nullptr && other.m_resource != nullptr)
	{
		return strcmp(m_resource, other.m_resource) == 0;
	}
	else
	{
		return false;
	}
}

const char* String::get() const noexcept
{
	return m_resource;
}

void String::reserve(size_t len)
{
	if (m_resource == nullptr)
	{
		m_size = len;
		m_resource = new char[len];
	}
	else
	{
		throw("The string is not empty. Cannot reserve space");
	}
}

String String::substr(size_t from, size_t to)
{
	if (from > m_size)
	{
		throw("FROM is out of range");
	}
	else if (to > m_size)
	{
		throw("TO is out of range");
	}
	else if (to < from)
	{
		throw("TO lesser that FROM");
	}

	String result(to - from);
	for (size_t i = from; i < to; ++i)
	{
		result.m_resource[i - from] = m_resource[i];
	}

	return result;
}

size_t String::length() const noexcept
{
	return m_size;
}

void String::clear() noexcept
{
	memset(m_resource, '\0', m_size);
	m_size = 0;
}

void String::concat(const String& other)
{
	m_size += other.m_size;

#ifdef __linux__
	strcat(m_resource, other.get());
#elif defined _WIN32
	strcat_s(m_resource, other.m_size, other.get());
#endif
}

bool String::found(const String& other) const noexcept
{
	return strstr(m_resource, other.m_resource) != nullptr;
}

void String::newstrcpy(char** dest, const char* source)
{
#ifdef __linux__
	strcpy(*dest, source);
#elif defined _WIN32
	strcpy_s(*dest, strlen(source), source);
#endif
}

void String::reverse() noexcept
{
	m_resource = strrev(m_resource);
}

#ifdef __linux__
char* String::strrev(char * const str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}
#endif
