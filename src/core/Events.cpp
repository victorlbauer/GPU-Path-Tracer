#include <PT.h>
#include "Events.h"

namespace PT
{
	namespace
	{
		EventHandler handlers;
	}

	ResetAccumulatorEvent::ResetAccumulatorEvent(double&& timeNow) : time(std::move(timeNow))
	{
		type = EventType::ResetAccumulator;
	}

	CloseAppEvent::CloseAppEvent()
	{
		type = EventType::CloseApp;
	}

	CameraZoomEvent::CameraZoomEvent(float&& t_zoom) : zoom(std::move(t_zoom))
	{
		type = EventType::CameraZoom;
	}

	CameraDollyEvent::CameraDollyEvent(double& t_yoffset, float& t_deltaTime) : yoffset(std::move(t_yoffset)), deltaTime(t_deltaTime)
	{
		type = EventType::CameraDolly;
	}

	CameraPanEvent::CameraPanEvent(float& t_xoffset, float& t_yoffset, float& t_deltaTime) : xoffset(std::move(t_xoffset)), yoffset(std::move(t_yoffset)), deltaTime(t_deltaTime)
	{
		type = EventType::CameraPan;
	}

	CameraOrbitEvent::CameraOrbitEvent(float& t_xoffset, float& t_yoffset, float& t_deltaTime) : xoffset(std::move(t_xoffset)), yoffset(std::move(t_yoffset)), deltaTime(t_deltaTime)
	{
		type = EventType::CameraOrbit;
	}

	MouseButtonStateEvent::MouseButtonStateEvent(const uint32_t&& t_idx, bool&& t_state) : idx(t_idx), state(std::move(t_state))
	{
		type = EventType::MouseButtonState;
	}

	void SetEventCallback(EventType etype, Handler handler)
	{
		handlers[etype].push_back(handler);
	}

	void DispatchEvent(Event& e)
	{
		if (!handlers[e.type].empty())
			for(auto i = 0; i < handlers[e.type].size(); ++i)
				std::invoke(handlers[e.type][i], &e);
	}
}