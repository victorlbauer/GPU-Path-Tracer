#pragma once
#define MIN_WORK_GROUP_INVOCATION_X	1024
#define MAX_WIDTH   1920
#define MAX_HEIGHT  1080
#define MAX_BOUNCES 8

#define UNIFORM_BUFFER_BINDING_INDEX   0
#define DISPATCH_BUFFER_BINDING_INDEX  1
#define ATOMIC_BUFFER_BINDING_INDEX	   2
#define INEXTEND_BUFFER_BINDING_INDEX  3
#define OUTEXTEND_BUFFER_BINDING_INDEX 4
#define SHADOW_BUFFER_BINDING_INDEX	   5
#define HIT_BUFFER_BINDING_INDEX	   6
#define PATH_BUFFER_BINDING_INDEX	   7
#define SCENE_BUFFER_BINDING_INDEX	   8

namespace PT
{
	struct Atomics
	{
		Atomics() = delete;

		alignas(4) uint32_t extendWorkGroup;
		alignas(4) uint32_t shadeWorkGroup;
		alignas(4) uint32_t connectWorkGroup;
		alignas(4) uint32_t extendThreadCounter;
		alignas(4) uint32_t shadeThreadCounter;
		alignas(4) uint32_t connectThreadCounter;
	};

	struct Uniforms
	{
		Uniforms() = delete;

		alignas(16) glm::mat4x4 camView;
		alignas(16) glm::vec3 camWorldPos;
		alignas(4) float FOV;
		alignas(4) uint32_t nSphereLights;
		alignas(4) uint32_t nSpheres;
		alignas(4) uint32_t nModels;
		alignas(4) uint32_t frame;
	};

	struct alignas(16) BsdfSample
	{
		BsdfSample() = delete;

		alignas(16) glm::vec3 f;
		alignas(4)	float pdf;
	};

	struct alignas(16) LightSample
	{
		LightSample() = delete;

		alignas(16) glm::vec3 L;
		alignas(4)	float pdf;
		alignas(16) glm::vec3 emission;
		alignas(4)  bool inShadow;
	};

	struct alignas(16) PathState
	{
		PathState() = delete;

		alignas(16) glm::vec3 throughput;
		alignas(16) glm::vec3 radiance;
		alignas(4)	float mediumIOR;
		alignas(16)	BsdfSample bsdfSample;
		alignas(16) LightSample lightSample;
	};

	struct alignas(16) Hit
	{
		Hit() = delete;

		alignas(16) glm::vec3 point;
		alignas(4)  float t;
		alignas(16)	glm::vec3 lastPoint;
		alignas(4)  bool emitter;
		alignas(16) glm::vec3 N;
		alignas(4)  uint32_t matid;
		alignas(16) glm::vec3 V;
	};

	struct alignas(16) Ray 
	{
		Ray() = delete;

		alignas(16) glm::vec3 origin;
		alignas(16) glm::vec3 dir;
		alignas(4)	uint32_t pathid;
	};

	struct alignas(16) RayBuffer
	{
		RayBuffer() = delete;

		Ray ray;
	};


	struct AccumulatorProfiler
	{
		bool reset		 = false;
		bool resetTimer  = false;
		double lastTime	 = 0.0;
		double totalTime = 0.0;
		double timeLimit = 0.25;
	};

	class VAO
	{
		public:
			explicit VAO() = default;
			~VAO();

			void Init();

			void Bind();
			void Unbind();

		private:
			uint32_t m_id;
	};

	class GLBuffer
	{
		public:
			explicit GLBuffer() = default;
			~GLBuffer();

			void InitBuffer(const uint32_t& type, const uint32_t& storageType);

			void Bind();
			void Unbind();

			void InitData(const size_t& size, const uint32_t&& n, const uint32_t&& bufferIndex);

			template<typename T>
			void LoadData(const T& data, const size_t&& offset, const uint32_t&& n)
			{
				glBufferSubData(m_type, offset, sizeof(data) * n, &data);
			}

			template<typename T>
			void LoadData(const T& data, const size_t&& offset)
			{
				glBufferSubData(m_type, offset, sizeof(data), &data);
			}

			template<typename T>
			void GetData(const uint32_t&& offset, const size_t&& dataSize, const T& data)
			{
				glGetBufferSubData(m_type, offset, dataSize, data);
			}

		private:
			uint32_t m_id;
			uint32_t m_type;
			uint32_t m_storageType;
	};
}