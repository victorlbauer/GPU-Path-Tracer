#pragma once

namespace PT
{
	struct alignas(16) Material
	{
		explicit Material() :
			roughness(0.01f),
			metalness(0.0f),
			specular(0.5f),
			specularTint(0.0f),
			sheen(0.0f),
			sheenTint(0.0f),
			clearCoat(0.0f),
			clearCoatRoughness(0.0f),
			transmission(0.0f),
			IOR(0.0f),
			subsurface(0.0f),
			baseColor(glm::vec3(0.025, 0.08, 0.25)),
			density(glm::vec3(0.0f)),
			emission(glm::vec3(0.0f))
		{}

		explicit Material(float&& t_roughness,
						  float&& t_metalness,
						  float&& t_specular,
						  float&& t_specularTint,
						  float&& t_sheen,
						  float&& t_sheenTint,
						  float&& t_clearCoat,
						  float&& t_clearCoatRoughness,
						  float&& t_transmission,
						  float&& t_IOR,
						  float&& t_subsurface,
						  glm::vec3&& t_baseColor,
						  glm::vec3&& t_density,
						  glm::vec3&& t_emission)
		{
			roughness		   = std::move(t_roughness);
			metalness		   = std::move(t_metalness);
			specular		   = std::move(t_specular);
			specularTint	   = std::move(t_specularTint);
			sheen			   = std::move(t_sheen);
			sheenTint		   = std::move(t_sheenTint);
			clearCoat		   = std::move(t_clearCoat);
			clearCoatRoughness = std::move(t_clearCoatRoughness);
			transmission	   = std::move(t_transmission);
			IOR				   = std::move(t_IOR);
			subsurface		   = std::move(t_subsurface);
			baseColor		   = std::move(t_baseColor);
			density			   = std::move(t_density);
			emission		   = std::move(t_emission);
		}

		glm::vec3 baseColor;
		float roughness;
		glm::vec3 density;
		float metalness;
		glm::vec3 emission;
		float specular;
		float specularTint;
		float sheen;
		float sheenTint;
		float clearCoat;
		float clearCoatRoughness;
		float transmission;
		float IOR;
		float subsurface;
	};

	struct alignas(16) Sphere
	{
		explicit Sphere() = default;

		explicit Sphere(glm::vec3&& t_worldPos, float&& t_radius, uint32_t&& t_matid) : _pad(0.0f)
		{
			worldPos = std::move(t_worldPos);
			radius   = std::move(t_radius);
			matid	 = std::move(t_matid);
		}

		glm::vec3 worldPos; 
		float _pad;
		float radius;
		uint32_t matid;
	};

	struct alignas(16) SphereLight
	{	
		SphereLight(glm::vec3&& t_emittance, glm::vec3&& t_worldPos, float&& t_radius)
		{
			emittance = std::move(t_emittance);
			worldPos  = std::move(t_worldPos);
			radius	  = std::move(t_radius);
			area = 4.0f * glm::pi<float>() * radius * radius;
		}

		glm::vec3 emittance;
		float radius;
		glm::vec3 worldPos;
		float area;
	};
}