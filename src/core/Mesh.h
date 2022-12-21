#pragma once
#include "Logger.h"

#define MAX_TRIANGLES 100000
#define MAX_NODES	  100000

namespace PT
{	
	using Index = uint32_t;

	struct Bounds
	{
		glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

		glm::vec3 Diagonal() const;
		float SurfaceArea() const;
		void Union(const Bounds& other);
	};

	struct alignas(16) Vertex
	{
		alignas(16) glm::vec3 localPos = glm::vec3(0.0f);
		alignas(16) glm::vec3 normal = glm::vec3(0.0f);
	};

	struct Triangle
	{
		Triangle() : centroid(glm::vec3(0.0f)) {}

		Bounds bounds;
		glm::vec3 centroid;
		std::array<Vertex, 3> verts;
	};

	struct GPUTriangle
	{
		GPUTriangle(const std::array<Vertex, 3>& vertex) : verts(vertex) {}
		std::array<Vertex, 3> verts;
	};

	struct Transform
	{
		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 rotation	  = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 scaling	  = glm::vec3(1.0f, 1.0f, 1.0f);
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<Index> indices;
	};

	enum class SplitAxis { X, Y, Z };
	
	constexpr uint32_t minPrimitives = 2;

	class BVHNode
	{
		public:
			BVHNode(std::vector<Triangle>& triangles);
			BVHNode(uint32_t&& t_depth);
			~BVHNode();

			void FindBounds();
			void Subdivide();
			void Partition();

		public:
			bool isLeaf;
			uint32_t depth;
			uint32_t nChild;
			Bounds bounds;
			std::vector<Triangle*> primitives;
			BVHNode* left;
			BVHNode* right;

		private:
			glm::vec3 Offset(const glm::vec3& point) const;
			std::vector<Triangle*>::iterator SAHSplit();
	};

	struct alignas(16) GPUBVHNode
	{
		explicit GPUBVHNode(const BVHNode * node, uint32_t & n, const uint32_t & idx);

		alignas(16) glm::vec3 boundMin;
		alignas(4)  uint32_t secondChildOffset;
		alignas(16) glm::vec3 boundMax;
		alignas(4)  uint32_t nPrimitives;
	};

	class Model
	{
		public:
			explicit Model(const std::string&& filePath, Transform& transfor, uint32_t&& matid);
			void ApplyTransform(Vertex& vert) const;
			void BuildBVH();

		public:
			// Host side
			std::vector<Mesh> meshes;
			std::vector<Triangle> triangles;
			Transform transform;
		
			// Device side
			std::vector<GPUTriangle> gpuTriangles;
			std::vector<GPUBVHNode> gpuNodes;
			uint32_t matid;
	};

	struct alignas(16) GPUModel
	{
		GPUModel() = delete;

		GPUTriangle triangles[MAX_TRIANGLES];
		GPUBVHNode bvhnodes[MAX_NODES];
		uint32_t matid;
		uint32_t _pad[3];
	};
}