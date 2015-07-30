#pragma once

// Makes a derived class non-copyable
// class X : private NonCopyable
class NonCopyable
{
protected:
	NonCopyable() = default;
	~NonCopyable() = default;

public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};
