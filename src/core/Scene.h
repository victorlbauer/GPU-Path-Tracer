#pragma once
#include "Entity.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "Logger.h"

#define MAX_MATERIALS	   64
#define MAX_SPHERE_LIGHTS  16
#define MAX_SPHERES		   64
#define MAX_MODELS		   8


namespace PT
{
	constexpr size_t sceneBufferSize =
		sizeof(Material) * MAX_MATERIALS +
		sizeof(SphereLight) * MAX_SPHERE_LIGHTS +
		sizeof(Sphere) * MAX_SPHERES +
		sizeof(GPUModel) * MAX_MODELS;

	class Scene 
	{
		public:
			explicit Scene() = default;
			explicit Scene(const std::string& filePath);
			~Scene();

		public:
			void LoadScene();

		public:
			PerspectiveCamera* camera;

			Texture HDRItexture;
			std::vector<Material> materials;
			std::vector<SphereLight> sphereLights;
			std::vector<Sphere> spheres;
			std::vector<Texture> textures;
			std::vector<Model> models;
	};
}