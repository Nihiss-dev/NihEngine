#pragma once

class NonCopyable
{
public:
	NonCopyable(const NonCopyable& other) = delete;
	NonCopyable(NonCopyable&& other) = delete;
	NonCopyable& operator=(const NonCopyable& other) = delete;
	NonCopyable& operator=(NonCopyable&& other) = delete;

protected:
	NonCopyable() = default;
	~NonCopyable() = default;
};
