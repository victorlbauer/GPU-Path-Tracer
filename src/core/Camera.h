#pragma once
#include "Events.h"
#include "Logger.h"

namespace PT
{
	class Camera
	{
		public:
			virtual ~Camera() = default;

			virtual void Dolly(const double& yoffset, const float& deltaTime);
			virtual void Pan(const float& xoffset, const float& yoffset, const float& deltaTime);
			virtual void Orbit(const float& xoffset, const float& yoffset, const float& deltaTime);

			virtual const glm::vec3 GetWorldPosition() const;
			virtual const glm::mat4x4 GetView() const;
			void SetResolution(const uint32_t& width, const uint32_t& height);

			virtual void OnEvent(Event* e) = 0;

		protected:
			explicit Camera() = default;
			void UpdateVectors();

		protected:
			float m_eta;
			glm::mat4x4 m_view;
			glm::vec3 m_worldPos;
			glm::vec3 m_target;

			glm::vec3 m_front;
			glm::vec3 m_right;
			glm::vec3 m_up;

			uint32_t m_viewPortWidth, m_viewPortHeight;
	};

	class PerspectiveCamera : public Camera
	{
		public:
			explicit PerspectiveCamera(glm::vec3&& worldPos, glm::vec3&& target, float&& fov);

			void Zoom(const float& zoom);
			void OnEvent(Event* e) override;
			const float& GetFieldOfView() const;

		private:
			float m_fieldOfView;
	};
}