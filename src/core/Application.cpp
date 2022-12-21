#include <PT.h>
#include "Application.h"

namespace PT 
{
	Application::Application() : m_running(false), m_spp(0), m_window(nullptr) {}

	void Application::Init()
	{
		LOG_INFO("Initializing Application...\n");

		// Initialize GLFW
		if (!glfwInit()) 
		{
			LOG_CRITICAL("Failed to initialize GLFW!\n");
			exit(-1);
		}

		// Support for OpenGL 4.3+
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Read settings
		Settings settings;

		// Create a GLFW window
		try 
		{
			m_window = new Window("Path Tracer", settings.videoSettings.width, settings.videoSettings.height);
		}
		catch (const std::string errmsg)
		{
			LOG_CRITICAL(errmsg); 
			exit(-1);
		}

		// Initialize GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LOG_CRITICAL("Failed to initialize GLAD!\n");
			exit(-1);
		}

		Input::Init(settings, *m_window);
		Renderer::Init(settings, *m_window);

		SetEventCallback(EventType::CloseApp, [&](Event* e) { OnEvent(e); });
		SetEventCallback(EventType::ResetAccumulator, [&](Event* e) { OnEvent(e); });

		LOG_INFO("Application initialized successfully!\n");
	}

	void Application::Run()
	{
		float currentTime = 0.0;
		float lastTime = 0.0;
		float delta_t = 0.0;
		m_running = true;

		while (m_running)
		{
			currentTime = glfwGetTime();
			delta_t = currentTime - lastTime;
			lastTime = currentTime;

			Input::Update(delta_t);
			Renderer::Update();
			m_window->UpdateTitle(uint32_t(1.0 / delta_t), m_spp++);

			glfwPollEvents();
			glfwSwapBuffers(*m_window);
		}
	}

	void Application::Shutdown()
	{
		LOG_INFO("Shutting down Application...\n");
		Renderer::Shutdown();
		delete (m_window);
		LOG_INFO("Application shutted down successfully!\n");
	}

	void Application::OnEvent(Event* e)
	{
		switch (e->type)
		{
			case EventType::CloseApp:
				LOG_INFO("Close Application Event triggered.\n");
				m_running = false;
				break;
			case EventType::ResetAccumulator:
				m_spp = 0;
				break;
			default:
				LOG_WARNING("Application does not support this kind of event!\n");
				break;
		}
	}
}