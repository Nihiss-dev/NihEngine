#pragma once

#include <memory>
#include "Core/NonCopyable.h"

// TODO : make this thread safe
// TODO : make this more efficient by using a unique_ptr instead of shared_ptr, but that would require some changes to the way we use singletons in the engine

template <class T>
class Singleton : public NonCopyable
{
public:
	static std::shared_ptr<T> GetInstance();
	void Kill();

protected:
	static std::shared_ptr<T> m_Instance;
};

template<class T>
std::shared_ptr<T> Singleton<T>::m_Instance = nullptr;

template<class T>
std::shared_ptr<T> Singleton<T>::GetInstance()
{
	if (m_Instance == nullptr)
	{
		m_Instance = std::make_shared<T>();
	}
	return m_Instance;
}

template<class T>
void Singleton<T>::Kill()
{
	m_Instance.reset();
}