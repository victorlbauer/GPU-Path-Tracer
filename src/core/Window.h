#pragma once
#include "Logger.h"
#include "Events.h"

namespace PT
{
	class Window final
	{
		public:
			explicit Window(std::string&& title, uint32_t width, uint32_t height);
			~Window();

			void UpdateTitle(const uint32_t& fps, const uint32_t& spp) const;

			operator GLFWwindow* ();

		private:
			GLFWwindow* m_instance;
			std::string m_title;
			uint32_t m_width;
			uint32_t m_height;
	};
}