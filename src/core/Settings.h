#pragma once
#include <PT.h>

namespace PT
{
	struct VideoSettings
	{
		uint32_t width  = 1280;
		uint32_t height = 720;
		double fov		= 60.0;
		double gamma	= 2.2;
	};

	struct Settings
	{
		VideoSettings videoSettings;
	};
}