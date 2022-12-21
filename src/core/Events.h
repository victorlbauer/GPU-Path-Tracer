#pragma once

namespace PT 
{
	enum class EventType { None, ResetAccumulator, CloseApp, CameraZoom, CameraDolly, CameraPan, CameraOrbit, MouseButtonState };

	struct Event 
	{
		virtual ~Event() = default;
		EventType type;
	};

	struct ResetAccumulatorEvent : public Event
	{
		explicit ResetAccumulatorEvent(double&& timeNow);
		double time;
	};

	struct CloseAppEvent : public Event
	{
		explicit CloseAppEvent();
	};

	struct CameraZoomEvent : public Event
	{
		explicit CameraZoomEvent(float&& t_zoom);
		float zoom;
	};

	struct CameraDollyEvent : public Event
	{
		explicit CameraDollyEvent(double& t_yoffset, float& t_deltaTime);
		double yoffset;
		float deltaTime;
	};

	struct CameraPanEvent : public Event
	{
		explicit CameraPanEvent(float& t_xoffset, float& t_yoffset, float& t_deltaTime);
		float xoffset, yoffset;
		float deltaTime;
	};

	struct CameraOrbitEvent : public Event
	{
		explicit CameraOrbitEvent(float& t_xoffset, float& y_offset, float& t_deltaTime);
		float xoffset, yoffset;
		float deltaTime;
	};

	struct MouseButtonStateEvent : public Event
	{
		explicit MouseButtonStateEvent(const uint32_t&& t_idx, bool&& t_state);
		uint32_t idx;
		bool state;
	};

	using Handler = std::function<void(Event* e)>;
	using EventHandler = std::map<EventType, std::vector<Handler>>;

	void SetEventCallback(EventType etype, Handler handler);
	void DispatchEvent(Event& e);

	template<typename e, typename ... Args>
	void NewEvent(Args&& ... args)
	{
		auto newEvent = e(std::forward<Args>(args) ...);
		DispatchEvent(newEvent);
	}
}