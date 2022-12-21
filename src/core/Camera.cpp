#include <PT.h>
#include "Camera.h"

namespace PT
{
	namespace
	{
		constexpr float minFOV		  = 30.0f;
		constexpr float maxFOV		  = 90.0f;
		constexpr float minTargetDist = 1.0f;
		constexpr float maxTargetDist = 30.0f;
		constexpr float maxDollySpeed = 100.0f;
		constexpr float panSpeedX	  = 1.0f;
		constexpr float panSpeedY	  = 1.0f;
		constexpr float rotateSpeedX  = 150.0f;
		constexpr float rotateSpeedY  = 150.0f;
		constexpr glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
	}

	// === Standard abstract camera ===
	void Camera::Dolly(const double& yoffset, const float& deltaTime)
	{
		float dist = glm::distance(m_worldPos, m_target);	
		float relativeSpeed = std::min(glm::pow(dist, 2.0f), maxDollySpeed);
		m_worldPos += deltaTime * relativeSpeed * m_front * float(yoffset);
		UpdateVectors();
	}

	void Camera::Pan(const float& xoffset, const float& yoffset, const float& deltaTime) 
	{
		m_target   += deltaTime * panSpeedX * (m_right * xoffset +  m_up * yoffset);
		m_worldPos += deltaTime * panSpeedY * (m_right * xoffset +  m_up * yoffset);
		UpdateVectors();
	}

	void Camera::Orbit(const float& xoffset, const float& yoffset, const float& deltaTime)
	{
		// Translate to origin
		m_worldPos -= m_target;
	
		// Rotation angles
		float angleX = glm::two_pi<float>() * deltaTime * rotateSpeedX * xoffset / m_viewPortWidth;
		float angleY = glm::two_pi<float>() * deltaTime * rotateSpeedY * yoffset / m_viewPortHeight;

		// Don't let the camera look straight up or down
		if (glm::abs(glm::dot(m_front, worldUp) + angleY) > 0.999f)
			angleY = 0.0f;

		// Rotate
		glm::mat4 rotation(1.0f); 
		rotation = glm::rotate(rotation, angleX, worldUp);
		rotation = glm::rotate(rotation,-angleY, m_right);
		m_worldPos = glm::vec3(rotation * glm::vec4(m_worldPos, 1.0f));

		// Translate back
		m_worldPos += m_target;
		UpdateVectors();
	}

	const glm::vec3 Camera::GetWorldPosition() const
	{
		return m_worldPos;
	}

	const glm::mat4x4 Camera::GetView() const
	{
		return m_view;
	}

	void Camera::SetResolution(const uint32_t& width, const uint32_t& height)
	{
		m_viewPortWidth = width;
		m_viewPortHeight = height;
	}

	void Camera::UpdateVectors()
	{
		m_front = glm::normalize(m_target - m_worldPos);
		m_right = glm::normalize(glm::cross(worldUp, m_front));
		m_up	= glm::normalize(glm::cross(m_front, m_right));
		m_view  = glm::lookAt(m_worldPos, m_target, worldUp);
	}

	PerspectiveCamera::PerspectiveCamera(glm::vec3&& worldPos, glm::vec3&& target, float&& fov)
	{
		m_worldPos = std::move(worldPos);
		m_target = std::move(target);
		m_fieldOfView = std::move(fov);

		UpdateVectors();

		SetEventCallback(EventType::CameraZoom,	 [&](Event* e) { OnEvent(e); });
		SetEventCallback(EventType::CameraDolly, [&](Event* e) { OnEvent(e); });
		SetEventCallback(EventType::CameraPan,   [&](Event* e) { OnEvent(e); });
		SetEventCallback(EventType::CameraOrbit, [&](Event* e) { OnEvent(e); });
	}

	void PerspectiveCamera::Zoom(const float& zoom)
	{
		m_fieldOfView += zoom;
		m_fieldOfView = std::min(std::max(m_fieldOfView, minFOV), maxFOV);
	}

	void PerspectiveCamera::OnEvent(Event* e)
	{
		switch (e->type)
		{
			case EventType::CameraZoom: 
			{
				auto zoomEvent = static_cast<CameraZoomEvent*>(e);
				Zoom(zoomEvent->zoom);
				break;
			}
			case EventType::CameraDolly:
			{
				auto dollyEvent = static_cast<CameraDollyEvent*>(e);
				Dolly(dollyEvent->yoffset, dollyEvent->deltaTime);
				break;
			}
			case EventType::CameraPan:
			{
				auto panEvent = static_cast<CameraPanEvent*>(e);
				Pan(panEvent->xoffset, panEvent->yoffset, panEvent->deltaTime);
				break;
			}
			case EventType::CameraOrbit:
			{
				auto orbitEvent = static_cast<CameraOrbitEvent*>(e);
				Orbit(orbitEvent->xoffset, orbitEvent->yoffset, orbitEvent->deltaTime);
				break;
			}
			default:
				LOG_WARNING("Perspective Camera does not support this kind of event!\n");
		}
	}

	const float& PerspectiveCamera::GetFieldOfView() const
	{
		return m_fieldOfView;
	}
}