#ifndef __STRING__H__
#define __STRING__H__

#include <cstdlib>

class String
{
public:
	String();
	String(size_t len);
	String(const char* other);
	String(const String& other);
	~String();

	char& operator [] (size_t idx);
	bool operator != (const String& other) const noexcept;
	bool operator == (const String& other) const noexcept;
	String& operator = (const String& other);
	String& operator = (const char* other);

	const char* get() const noexcept;
	void reserve(size_t len);
	String substr(size_t from, size_t to);
	size_t length() const noexcept;
	void clear() noexcept;
	void concat(const String& other);
	bool found(const String& other) const noexcept;
	void reverse() noexcept;

private:
#ifdef __linux__
	char* strrev(char * const);
#endif

	void newstrcpy(char** dest, const char* source);
	size_t m_size;
	char* m_resource = nullptr;
};

#endif // __STRING__H__
