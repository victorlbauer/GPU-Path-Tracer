#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Buffer.h"
#include "Profiler.h"
#include "Settings.h"
#include "Entity.h"
#include "Scene.h"
#include "Events.h"
#include "Window.h"

#define OUTPUT_TEX_BINDING		0
#define ACCUMULATOR_TEX_BINDING 1
#define SCENE_TEX_BINDING		2

namespace PT::Renderer
{
	void Init(const Settings& settings, Window& window);
	void Update();
	void Shutdown();

	namespace
	{
		inline uint32_t GetNumWorkGroups(const uint32_t& nwork)
		{
			return std::max<uint32_t>(1, (uint32_t)glm::ceil((float)nwork / (float)MIN_WORK_GROUP_INVOCATION_X));
		}

		void SetDynamicUniforms();
		void ResetWorkBuffers();
		void ResetAccumulator();
		void SwapBuffers();
		void OnEvent(Event* e);
		void LoadScene(const std::string& filePath);
	}
}