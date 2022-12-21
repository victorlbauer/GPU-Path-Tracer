#pragma once
#include "Logger.h"
#include "Events.h"
#include "Window.h"
#include "Settings.h"

#define KEY_PRESSED(key)	Input::commandQueue.emplace(keys[key].OnKeyPress())
#define KEY_REPEATED(key)	Input::commandQueue.emplace(keys[key].OnKeyPress())
#define KEY_RELEASED(key)	Input::commandQueue.emplace(keys[key].OnKeyRelease())

#define BUTTON_PRESSED(button)	Input::commandQueue.emplace(mouse.OnButtonPress(button))
#define BUTTON_RELEASED(button)	Input::commandQueue.emplace(mouse.OnButtonRelease(button))

namespace PT::Input
{
	void Init(const Settings& settings, Window& window);
	void Update(const float& delta_t);
}