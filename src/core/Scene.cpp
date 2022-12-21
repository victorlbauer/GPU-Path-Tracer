#include <PT.h>
#include "Scene.h"

namespace PT
{
	Scene::Scene(const std::string& filePath)
	{
		LoadScene();
	}

	Scene::~Scene()
	{
		delete(camera);
	}

	void Scene::LoadScene()
	{
		camera = new PerspectiveCamera(glm::vec3(0.0f, 15.0f, -20.0f), glm::vec3(0.0f, 0.0f, 1.0f), 60.0f);

		//		    roughness metal  spec  specT sheen sheenT clear clearR trans  IOR   SSS
		Material mat1(0.100f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.000f, 0.0f, 1.00f, 0.0f, glm::vec3(1.00f, 0.00f, 0.00f), glm::vec3(0.0f), glm::vec3(0.0f));
		Material mat2(1.000f, 1.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.000f, 0.0f, 1.00f, 0.0f, glm::vec3(0.05f, 0.25f, 0.80f), glm::vec3(0.0f), glm::vec3(0.0f));
		materials.emplace_back(mat1); materials.emplace_back(mat2); 

		Sphere s(glm::vec3(0.0f), 1.0f, 0);	
		//spheres.emplace_back(s);

		SphereLight sl(glm::vec3(50.0), glm::vec3(5.0, 10.0, -5.0), 2.0);
		sphereLights.emplace_back(sl);

		HDRItexture = Texture("resources/assets/textures/hdri/apartment.hdr");

		Transform transform;
		transform.translation = glm::vec3(0.0f, -1.0f, 0.0f);
		transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		transform.scaling = glm::vec3(5.0f, 0.05f, 5.0f);

		Model* model = new Model("resources/assets/meshes/obj/cube.obj", transform, 1);
		models.emplace_back(std::move(*model));
		delete model;

		transform.translation = glm::vec3(0.0f, 0.25f, 0.0f);
		transform.rotation = glm::vec3(30.0f, 180.0f, 0.0f);
		transform.scaling = glm::vec3(2.0f);
		Model* suzanne = new Model("resources/assets/meshes/obj/suzanne.obj", transform, 0);
		models.emplace_back(std::move(*suzanne));
		delete suzanne;

		LOG_INFO("Building the BVH...");
		for (size_t i = 0; i < models.size(); ++i)
		{
			models[i].BuildBVH();
		}
		LOG("Done!\n");
	}
}