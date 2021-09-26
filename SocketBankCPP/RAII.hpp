#ifndef __RAII__HPP__
#define __RAII__HPP__

template <typename T>
class RAII
{
public:
	RAII() : m_resource(new T)
	{
		//
	}

	~RAII()
	{
		delete m_resource;
	}

	T* operator = (T* other)
	{
		m_resource = other;
		return m_resource;
	}

	T* get()
	{
		return m_resource;
	}

	const T* get() const
	{
		return m_resource;
	}

private:
	T* m_resource;
};

#endif // __RAII__HPP__
