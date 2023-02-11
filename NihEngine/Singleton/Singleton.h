#pragma once

#include <memory>

template <class T>
class Singleton
{
public:
	static std::shared_ptr<T> GetInstance();
	void Kill();

protected:
	static std::shared_ptr<T> m_Instance;

private:
	T& operator= (const T&) {}
};

template<class T>
std::shared_ptr<T> Singleton<T>::m_Instance = nullptr;

template<class T>
std::shared_ptr<T> Singleton<T>::GetInstance()
{
	if (m_Instance == nullptr)
	{
		//m_Instance = new T();
		m_Instance = std::make_shared<T>();
	}
	return m_Instance;
}

template<class T>
void Singleton<T>::Kill()
{
	delete m_Instance;
	m_Instance = nullptr;
}