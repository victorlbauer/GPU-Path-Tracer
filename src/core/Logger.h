#pragma once
#include <PT.h>

#define LOG(...)			PT::WriteToLog(__VA_ARGS__)
#define LOG_INFO(...)		PT::WriteToLog("[INFO]: ",     __VA_ARGS__)
#define LOG_WARNING(...)	PT::WriteToLog("[WARNING]: ",  __VA_ARGS__)
#define LOG_CRITICAL(...)	PT::WriteToLog("[CRITICAL]: ", __VA_ARGS__)

namespace PT
{
	template<typename Msg, typename ... Args> 
	void WriteToLog(Msg&& msg, Args&& ... args)
	{
		std::cout << msg;
		(WriteToLog(std::forward<Args>(args)), ...);
	}
}