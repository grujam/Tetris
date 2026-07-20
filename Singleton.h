#pragma once

template <typename T>
class ISingleton
{
public:
	ISingleton() {}
	virtual ~ISingleton() {}

public:
	static T* Get()
	{
		static T m_Singleton;
		return &m_Singleton;
	}

private:
	ISingleton(const ISingleton&) = delete;
	ISingleton& operator=(const ISingleton&) = delete;
};