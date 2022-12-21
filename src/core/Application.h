#pragma once
#include "Logger.h"
#include "Window.h"
#include "Events.h"
#include "Settings.h"
#include "Input.h"
#include "Renderer.h"

namespace PT
{
	class Application final 
	{
		public:
			explicit Application();

			void Init();
			void Run();
			void Shutdown();

			void OnEvent(Event* e);

		private:
			bool m_running;
			uint32_t m_spp;
			Window* m_window;
	};
}