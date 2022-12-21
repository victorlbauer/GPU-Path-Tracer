#include <PT.h>
#include "Input.h"

namespace PT::Input
{
	// === Commands ===
	namespace
	{
		using Command = std::function<void()>;

		void Exit() { NewEvent<CloseAppEvent>(); }

		void ResetAccumulator() { NewEvent<ResetAccumulatorEvent>(glfwGetTime()); }

		void CameraZoomIn()  { ResetAccumulator(); NewEvent<CameraZoomEvent>(-5.0f); }
		void CameraZoomOut() { ResetAccumulator(); NewEvent<CameraZoomEvent>( 5.0f); }
		void CameraDolly(double& yoffset, float& delta_t) { ResetAccumulator(); NewEvent<CameraDollyEvent>(yoffset, delta_t); }
		void CameraPan(float& xoffset, float& yoffset, float& delta_t) { ResetAccumulator(); NewEvent<CameraPanEvent>(xoffset, yoffset, delta_t); }
		void CameraOrbit(float& xoffset, float& yoffset, float& delta_t) { ResetAccumulator(); NewEvent<CameraOrbitEvent>(xoffset, yoffset, delta_t); }
	}

	// === Keyboard ===
	namespace
	{
		class Key
		{
			public:
				explicit Key()
				{
					m_onKeyPress = [] {};
					m_onKeyRelease = [] {};
				}

				void SetOnKeyPress(Command command)
				{
					m_onKeyPress = command;
				}

				void SetOnKeyRelease(Command command)
				{
					m_onKeyRelease = command;
				}

				inline Command OnKeyPress()
				{
					return m_onKeyPress;
				}

				inline Command OnKeyRelease()
				{
					return m_onKeyRelease;
				}

			protected:
				Command m_onKeyPress;
				Command m_onKeyRelease;
		};

		constexpr uint32_t MAX_KEYS = 1024;
		std::array<Key, MAX_KEYS> keys;

		void InitKeys()
		{
			keys[GLFW_KEY_ESCAPE].SetOnKeyPress(Exit);
			keys[GLFW_KEY_R].SetOnKeyPress(CameraZoomIn);
			keys[GLFW_KEY_F].SetOnKeyPress(CameraZoomOut);
		}
	}

	// === Mouse ===
	namespace
	{
		constexpr uint32_t MAX_BUTTONS = 8;
		
		class Button : public Key
		{
			public:
				explicit Button() : m_held(false) {}
				bool IsButtonPressed() const { return m_held; }
				bool m_held;
		};

		class Mouse
		{
			public:
				explicit Mouse() : m_sensitivity(1.0f), m_posX(0.0), m_posY(0.0) {}

				double GetMousePosX() const { return m_posX; }
				double GetMousePosY() const { return m_posY; }

				void SetMousePosition(double& posX, double& posY)
				{
					m_posX = posX;
					m_posY = posY;
				}

				void SetOnButtonPress(const uint32_t& idx, Command command)
				{
					assert(idx <= MAX_BUTTONS);
					m_buttons[idx].SetOnKeyPress(command);
				}

				void SetOnButtonRelease(const uint32_t& idx, Command command)
				{
					assert(idx <= MAX_BUTTONS);
					m_buttons[idx].SetOnKeyRelease(command);
				}

				Command OnButtonPress(const uint32_t& idx)
				{
					assert(idx <= MAX_BUTTONS);
					return m_buttons[idx].OnKeyPress();
				}

				Command OnButtonRelease(const uint32_t& idx)
				{
					assert(idx <= MAX_BUTTONS);
					return m_buttons[idx].OnKeyRelease();
				}

				bool IsButtonPressed(const uint32_t& idx)
				{
					assert(idx <= MAX_BUTTONS);
					return m_buttons[idx].IsButtonPressed();
				}

				void SetButtonState(const uint32_t& idx, bool& state)
				{
					assert(idx <= MAX_BUTTONS);
					m_buttons[idx].m_held = state;
				}

			private:
				float m_sensitivity;
				double m_posX, m_posY;
				std::array<Button, MAX_BUTTONS> m_buttons;
		};

		Mouse mouse;

		void InitButtons()
		{
			mouse.SetOnButtonPress(GLFW_MOUSE_BUTTON_LEFT,	  [] { NewEvent<MouseButtonStateEvent>(GLFW_MOUSE_BUTTON_LEFT,   true); });
			mouse.SetOnButtonPress(GLFW_MOUSE_BUTTON_RIGHT,	  [] { NewEvent<MouseButtonStateEvent>(GLFW_MOUSE_BUTTON_RIGHT,  true); });
			mouse.SetOnButtonPress(GLFW_MOUSE_BUTTON_MIDDLE,  [] { NewEvent<MouseButtonStateEvent>(GLFW_MOUSE_BUTTON_MIDDLE, true); });

			mouse.SetOnButtonRelease(GLFW_MOUSE_BUTTON_LEFT,  [] { NewEvent<MouseButtonStateEvent>(GLFW_MOUSE_BUTTON_LEFT,  false); });
			mouse.SetOnButtonRelease(GLFW_MOUSE_BUTTON_RIGHT, [] { NewEvent<MouseButtonStateEvent>(GLFW_MOUSE_BUTTON_RIGHT, false); });
			mouse.SetOnButtonRelease(GLFW_MOUSE_BUTTON_MIDDLE,[] { NewEvent<MouseButtonStateEvent>(GLFW_MOUSE_BUTTON_MIDDLE,false); });
		}
	}

	// === Input handler ===
	namespace
	{
		float s_deltaTime;
		std::queue<Command> commandQueue;

		void KeyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key > MAX_KEYS)
				return;

			switch (action)
			{
				case GLFW_PRESS:
					KEY_PRESSED(key);
					break;
				case GLFW_REPEAT:
					KEY_REPEATED(key);
					break;
				case GLFW_RELEASE:
					KEY_RELEASED(key);
					break;
				default:
					break;
			}
		}

		void MouseInputCallback(GLFWwindow* window, int button, int action, int mods)
		{
			if (button > MAX_BUTTONS)
				return;
			
			switch (action)
			{
				case GLFW_PRESS:
					BUTTON_PRESSED(button);
					break;
				case GLFW_RELEASE:
					BUTTON_RELEASED(button);
					break;
			}
		}

		void MousePosCallback(GLFWwindow* window, double xpos, double ypos)
		{
			glfwGetCursorPos(window, &xpos, &ypos);

			// Mouse repeat events
			if (mouse.IsButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				float xoffset = -(xpos - mouse.GetMousePosX());
				float yoffset = -(ypos - mouse.GetMousePosY());
				CameraOrbit(xoffset, yoffset, s_deltaTime);
			}

			if (mouse.IsButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) || mouse.IsButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
			{
				float xoffset = xpos - mouse.GetMousePosX();
				float yoffset = ypos - mouse.GetMousePosY();
				CameraPan(xoffset, yoffset, s_deltaTime);
			}

			mouse.SetMousePosition(xpos, ypos);
		}

		void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
		{
			CameraDolly(yoffset, s_deltaTime);
		}
	}

	namespace
	{
		void OnEvent(Event* e)
		{
			switch (e->type)
			{
				case EventType::MouseButtonState:
				{
					auto mouseEvent = static_cast<MouseButtonStateEvent*>(e);
					mouse.SetButtonState(mouseEvent->idx, mouseEvent->state);
				}
			}
		}
	}

	void Init(const Settings& settings, Window& window)
	{
		LOG_INFO("Initializing the Input System...\n");
		InitKeys();
		InitButtons();

		// GLFW callbacks
		glfwSetKeyCallback(window, KeyInputCallback);
		glfwSetMouseButtonCallback(window, MouseInputCallback);
		glfwSetCursorPosCallback(window, MousePosCallback);
		glfwSetScrollCallback(window, MouseScrollCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

		SetEventCallback(EventType::MouseButtonState, [&](Event* e) { OnEvent(e); });

		LOG_INFO("Input System initialized successfully!\n");
	}

	void Update(const float& deltaTime)
	{
		s_deltaTime = deltaTime;
		if (!commandQueue.empty())
		{
			for (auto i = 0; i < commandQueue.size(); ++i)
			{
				std::invoke(commandQueue.front());
				commandQueue.pop();
			}
		}
	}
}