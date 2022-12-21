#include <PT.h>
#include "Profiler.h"

namespace PT
{
	Timer::Timer() : m_nSamples(0), m_mean(0.0) {}

	void Timer::Stop()
	{
		m_endPoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startPoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_endPoint).time_since_epoch().count();

		auto duration = end - start;
		double ms = duration * 0.001;

		m_nSamples++;
		m_mean += ms;
	}

	void Timer::Start()
	{
		m_startPoint = std::chrono::high_resolution_clock::now();
	}

	double Timer::GetMean()
	{
		return double(m_mean / m_nSamples);
	}
}