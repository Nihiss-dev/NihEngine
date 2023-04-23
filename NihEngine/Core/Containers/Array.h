#pragma once

#include <array>
#include "System/Assert.h"

template <typename T, size_t size>
class Array
{
public:
	[[nodiscard]] constexpr const T& At(size_t pos) const
	{
		NIH_ASSERT(pos < size);
		return m_Data[pos];
	}
	[[nodiscard]] constexpr T& At(size_t pos)
	{
		NIH_ASSERT(pos < size);
		return m_Data[pos];
	}

	[[nodiscard]] constexpr size_t GetSize() const { return size; };
	[[nodiscard]] constexpr const T& Front() const { return m_Data[0]; }
	[[nodiscard]] constexpr T& Front() { return m_Data[0]; }
	[[nodiscard]] constexpr const T& Back() const { return m_Data[size - 1]; }
	[[nodiscard]] constexpr T& Back() { return m_Data[size - 1]; }
	[[nodiscard]] constexpr const T& operator[](size_t pos) const
	{
		NIH_ASSERT(pos < size);
		return m_Data[pos];
	}
	[[nodiscard]] constexpr T& operator[](size_t pos)
	{
		NIH_ASSERT(pos < size);
		return m_Data[pos];
	}

	T m_Data[size];
};

// Deducing guides that will let us write Array array{0,1,2,3} without specifying type or size
template<class Type, class... Size>
Array(Type, Size...) -> Array<Type, 1 + sizeof...(Size)>;
