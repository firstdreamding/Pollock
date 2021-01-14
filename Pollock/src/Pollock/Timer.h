#pragma once

#include <iostream>
#include <string>
#include <chrono>

class ScopedTimer
{
public:
	ScopedTimer(const std::string& name)
		: m_Name(name)
	{
		Reset();
	}

	~ScopedTimer()
	{
		std::cout << "[TIMER] " << m_Name << " - " << ElapsedMillis() << "ms\n";
	}

	void Reset()
	{
		m_Start = std::chrono::high_resolution_clock::now();
	}

	float Elapsed()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
	}

	float ElapsedMillis()
	{
		return Elapsed() * 1000.0f;
	}
private:
	std::string m_Name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};
