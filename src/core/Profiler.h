#pragma once
#include "Logger.h"

namespace PT
{
	class Timer final
	{
		public:
			explicit Timer();

			void Start();
			void Stop();

			double GetMean();

		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> m_startPoint;
			std::chrono::time_point<std::chrono::high_resolution_clock> m_endPoint;
			uint32_t m_nSamples;
			double m_mean;
	};
}