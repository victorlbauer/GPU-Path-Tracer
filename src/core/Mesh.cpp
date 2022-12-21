#include <PT.h>
#include "Mesh.h"

namespace PT
{
	glm::vec3 Bounds::Diagonal() const
	{
		return glm::clamp(max - min, 0.0f, std::numeric_limits<float>::max());
	}

	float Bounds::SurfaceArea() const
	{
		glm::vec3 d = Diagonal();
		return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	void Bounds::Union(const Bounds& other)
	{
		this->min = glm::vec3(std::min(this->min.x, other.min.x),
							  std::min(this->min.y, other.min.y),
							  std::min(this->min.z, other.min.z));

		this->max = glm::vec3(std::max(this->max.x, other.max.x),
							  std::max(this->max.y, other.max.y),
							  std::max(this->max.z, other.max.z));
	}

	BVHNode::BVHNode(std::vector<Triangle>& triangles) : depth(0),
						 nChild(0),
						 isLeaf(true), 
						 left(nullptr), 
						 right(nullptr) 
	{
		
		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			this->primitives.push_back(&triangles[i]);
		}
	}

	BVHNode::BVHNode(uint32_t&& t_depth) : depth(t_depth),
										   nChild(0),
										   isLeaf(true),
										   left(nullptr),
										   right(nullptr) {}

	BVHNode::~BVHNode()
	{
		if (this->left)
			delete(this->left);
		if (this->right) 
			delete(this->right);
	}

	void BVHNode::FindBounds()
	{
		for (Triangle* i : primitives)
		{
			this->bounds.Union(i->bounds);
		}
	}

	void BVHNode::Subdivide()
	{
		if (this->primitives.size() <= minPrimitives)
			return;

		this->left = new BVHNode(this->depth + 1);
		this->right = new BVHNode(this->depth + 1);

		this->nChild += 2;

		Partition();

		// Empty child, so this node couldn't be partitioned any further. Return it as a leaf instead
		if (this->left->primitives.size() == 0 || this->right->primitives.size() == 0)
		{
			this->nChild = 0;
			delete(this->left);
			delete(this->right);
			this->left = nullptr;
			this->right = nullptr;
			return;
		}

		this->left->FindBounds();
		this->right->FindBounds();

		this->left->Subdivide();
		this->right->Subdivide();

		this->nChild += left->nChild + right->nChild;
		this->isLeaf = false;
	}

	void BVHNode::Partition()
	{
		std::vector<Triangle*>::iterator ptr = SAHSplit();

		for (auto i = this->primitives.begin(); i < ptr; ++i)
		{
			this->left->primitives.emplace_back(*i);
		}

		for (auto i = ptr; i != this->primitives.end(); ++i)
		{
			this->right->primitives.emplace_back(*i);
		}
	}


	GPUBVHNode::GPUBVHNode(const BVHNode* node, uint32_t& n, const uint32_t& idx) : nPrimitives(0)
	{
		this->boundMin = node->bounds.min;
		this->boundMax = node->bounds.max;
	
		// Leaf node
		if (node->isLeaf)
		{
			this->secondChildOffset = n;
			this->nPrimitives = node->primitives.size();
			n += this->nPrimitives;
		}
		// Parent node
		else
		{
			this->secondChildOffset = idx + node->left->nChild + 2;
			this->nPrimitives = 0;
		}
	}

	glm::vec3 BVHNode::Offset(const glm::vec3& point) const
	{
		glm::vec3 offset = point - this->bounds.min;

		if (this->bounds.max.x >= point.x)
			offset.x /= this->bounds.max.x - this->bounds.min.x;
		if (this->bounds.max.y >= point.y)
			offset.y /= this->bounds.max.y - this->bounds.min.y;
		if (this->bounds.max.z >= point.z)
			offset.z /= this->bounds.max.z - this->bounds.min.z;

		return offset;
	}

	// As per PBR Book - Chapter 4.3
	std::vector<Triangle*>::iterator BVHNode::SAHSplit()
	{
		const size_t naxis = 3;
		const size_t nbins = 16;

		struct Bin
		{
			uint32_t count = 0;
			Bounds bounds;
		} bins[naxis][nbins];

		// See where each primitive lands on each bin
		for (size_t axis = 0; axis < naxis; ++axis) 
		{
			for (Triangle* i : this->primitives)
			{
				uint32_t bidx = uint32_t(nbins * Offset(i->centroid)[axis]);

				if (bidx == nbins)
					bidx = nbins - 1;

				bins[axis][bidx].count++;
				bins[axis][bidx].bounds.Union(i->bounds);
			}
		}

		// Calculate all bins costs
		float cost[naxis][nbins - 1];

		for (size_t axis = 0; axis < naxis; ++axis)
		{
			for (size_t i = 0; i < nbins - 1; ++i)
			{
				Bounds lower, upper;
				uint32_t countLower = 0;
				uint32_t countUpper = 0;

				for (size_t j = 0; j <= i; ++j)
				{
					lower.Union(bins[axis][j].bounds);
					countLower += bins[axis][j].count;
				}

				for (size_t j = i + 1; j < nbins; ++j)
				{
					upper.Union(bins[axis][j].bounds);
					countUpper += bins[axis][j].count;
				}

				cost[axis][i] = 0.125f + (countLower * lower.SurfaceArea() + countUpper * upper.SurfaceArea()) / this->bounds.SurfaceArea();
			}
		}

		// Find the split with the minimum cost
		float minCost = cost[0][0];
		size_t minCostSplitBin = 0, minCostAxis = 0;

		for (size_t axis = 0; axis < naxis; ++axis)
		{
			for (size_t i = 1; i < nbins - 1; ++i)
			{
				if (cost[axis][i] < minCost)
				{
					minCost = cost[axis][i];
					minCostSplitBin = i;
					minCostAxis = axis;
				}
			}
		}

		// Rearrange primitives according to the best split
		std::vector<Triangle*>::iterator ptr;
		float leafCost = float(this->primitives.size());

		// Splitting is not worth it, just turn this node into a leaf
		if (leafCost < minCost && this->primitives.size() > minPrimitives)
			ptr = this->primitives.end();
		// Split this node
		else
			ptr = std::partition(this->primitives.begin(), this->primitives.end(), [&](Triangle* t)
				{
					uint32_t bidx = uint32_t(nbins * Offset(t->centroid)[minCostAxis]);
					
					if (bidx == nbins)
						bidx = nbins - 1;

					return bidx <= minCostSplitBin;
				});

		return ptr;
	}

	// TODO: Change to fetch this data from the file
	Model::Model(const std::string&& filePath, Transform& transform, uint32_t&& matid)
	{
		LOG_INFO("Loading model at (", filePath, ")...");

		this->transform = transform;
		this->matid = matid;

		// Load OBJ model and its meshes
		objl::Loader loader;
		bool loadout = loader.LoadFile(filePath);

		if (loadout)
		{
			for (auto i = 0; i < loader.LoadedMeshes.size(); ++i)
			{
				Mesh* m = new Mesh();

				objl::Mesh currentMesh = loader.LoadedMeshes[i];

				for (auto j = 0; j < currentMesh.Vertices.size(); ++j)
				{
					Vertex vert;
					vert.localPos = std::move(glm::vec3(currentMesh.Vertices[j].Position.X, currentMesh.Vertices[j].Position.Y, currentMesh.Vertices[j].Position.Z));
					vert.normal = std::move(glm::vec3(currentMesh.Vertices[j].Normal.X, currentMesh.Vertices[j].Normal.Y, currentMesh.Vertices[j].Normal.Z));
					ApplyTransform(vert);
					m->vertices.emplace_back(std::move(vert));
				}

				for (auto j = 0; j < currentMesh.Indices.size(); ++j)
				{
					Index idx = currentMesh.Indices[j];
					m->indices.emplace_back(std::move(idx));
				}

				this->meshes.emplace_back(std::move(*m));
				delete m;
			}
		}

		// Create a list of primitives for the BVH construction
		if (!meshes.empty())
		{
			for (size_t i = 0; i < meshes.size(); ++i)
			{
				for (size_t j = 0; j < meshes[i].indices.size(); j += 3)
				{
					Triangle triangle;
					size_t e0 = meshes[i].indices[j];
					size_t e1 = meshes[i].indices[j+1];
					size_t e2 = meshes[i].indices[j+2];

					triangle.verts[0] = meshes[i].vertices[e0];
					triangle.verts[1] = meshes[i].vertices[e1];
					triangle.verts[2] = meshes[i].vertices[e2];
					
					// Find AABB bounds
					glm::vec3 min;
					min.x = std::min({ meshes[i].vertices[e0].localPos.x, meshes[i].vertices[e1].localPos.x, meshes[i].vertices[e2].localPos.x });
					min.y = std::min({ meshes[i].vertices[e0].localPos.y, meshes[i].vertices[e1].localPos.y, meshes[i].vertices[e2].localPos.y });
					min.z = std::min({ meshes[i].vertices[e0].localPos.z, meshes[i].vertices[e1].localPos.z, meshes[i].vertices[e2].localPos.z });

					glm::vec3 max;
					max.x = std::max({ meshes[i].vertices[e0].localPos.x, meshes[i].vertices[e1].localPos.x, meshes[i].vertices[e2].localPos.x });
					max.y = std::max({ meshes[i].vertices[e0].localPos.y, meshes[i].vertices[e1].localPos.y, meshes[i].vertices[e2].localPos.y });
					max.z = std::max({ meshes[i].vertices[e0].localPos.z, meshes[i].vertices[e1].localPos.z, meshes[i].vertices[e2].localPos.z });

					triangle.bounds.min = std::move(min);
					triangle.bounds.max = std::move(max);
					triangle.centroid = 0.5f * (triangle.bounds.max + triangle.bounds.min);

					this->triangles.emplace_back(std::move(triangle));
				}
			}
		}

		LOG(" Done!\n");
		LOG("\nModel's info:\n");
		LOG("\tTriangles: ", this->triangles.size(), "\n\n");
	}

	void Model::ApplyTransform(Vertex& vert) const
	{
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, this->transform.translation);
		trans = glm::rotate(trans, glm::radians(this->transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(this->transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(this->transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		trans = glm::scale(trans, this->transform.scaling);

		glm::mat4 transInv = glm::inverse(trans);

		vert.localPos = glm::vec3(trans * glm::vec4(vert.localPos, 1.0f));
		vert.normal = glm::normalize(glm::mat3(glm::transpose(transInv)) * vert.normal);
	}

	void Model::BuildBVH()
	{
		// Host side BVH
		BVHNode* root = new BVHNode(this->triangles);
		root->FindBounds();
		root->Subdivide();
		
		// Convert it to a linear layout for GPU traversal
		std::stack<BVHNode*> visited;
		visited.push(root);
		uint32_t n = 0;
		uint32_t ind = 0;

		while (!visited.empty())
		{
			BVHNode* current = visited.top();
			this->gpuNodes.emplace_back(std::move(GPUBVHNode(current, n, ind++)));
			visited.pop();

			if (current->right)
				visited.push(current->right);
			if (current->left)
				visited.push(current->left);

			if (current->isLeaf)
			{
				for (Triangle* i : current->primitives)
				{
					this->gpuTriangles.emplace_back(std::move(GPUTriangle(i->verts)));
				}
			}
		}

		delete root;
	}
}