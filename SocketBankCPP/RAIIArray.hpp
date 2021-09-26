#ifndef __RAII__ARRAY__HPP__
#define __RAII__ARRAY__HPP__

typedef unsigned long size_t;

template <typename Arr>
class RAIIArray
{
public:
	RAIIArray(size_t len) :
		m_size(len)
		,m_resource(new Arr[len])
	{
		//
	}

	~RAIIArray()
	{
		delete[] m_resource;
	}

	RAIIArray(RAIIArray& other) :
		m_size(other.m_size)
		,m_resource(new Arr[other.m_size])
	{
		for (size_t i = 0; i < m_size; ++i)
		{
			m_resource[i] = other[i];
		}
	}

	Arr* get()
	{
		return m_resource;
	}

	const Arr* get() const
	{
		return m_resource;
	}

	Arr* operator = (Arr* other)
	{
		if (other != nullptr)
		{
			Arr* newarr = new Arr[sizeof(other) / sizeof(other[0])];

			for (size_t i = 0; i < m_size; ++i)
			{
				newarr[i] = other[i];
			}
			
			delete[] m_resource;
			m_resource = newarr;
		}
		else
		{
			throw ("Cannot assign NULL");
		}

		return m_resource;
	}

	Arr& operator [] (size_t idx)
	{
		return m_resource[idx];
	}

private:
	size_t m_size;
	Arr* m_resource;
};

#endif // __RAII__ARRAY__HPP__
