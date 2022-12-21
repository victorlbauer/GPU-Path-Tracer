#include <PT.h>
#include "Window.h"

namespace PT 
{
	Window::Window(std::string&& title, uint32_t width, uint32_t height) : m_title(std::move(title)), m_width(std::move(width)), m_height(std::move(height))
	{
		m_instance = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
		if (m_instance == nullptr)
		{
			throw "FAILED TO CREATE A GLFW WINDOW!";
		}
		else
		{
			// GLFW callbacks
			glfwSetWindowSizeCallback(m_instance, [](GLFWwindow* window, int width, int height) 
			{ 
				glViewport(0, 0, width, height); 
			});

			glfwSetWindowCloseCallback(m_instance, [](GLFWwindow* window) 
			{
				NewEvent<CloseAppEvent>();
			});

			// Windows API to get the screen's resolution
			DWORD screenWidth  = GetSystemMetrics(SM_CXSCREEN);
			DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
			glfwSetWindowPos(m_instance, (screenWidth - m_width) / 2, (screenHeight - m_height) / 2);
			
			glfwSetInputMode(m_instance, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			glfwMakeContextCurrent(m_instance);
		}
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_instance);
	}

	void Window::UpdateTitle(const uint32_t& fps, const uint32_t& spp) const
	{
		std::string title = m_title + " (" + std::to_string(fps) + " FPS) - " + std::to_string(spp) + " [spp]";
		glfwSetWindowTitle(m_instance, title.c_str());
	}

	Window::operator GLFWwindow* () 
	{
		return m_instance;
	}
}