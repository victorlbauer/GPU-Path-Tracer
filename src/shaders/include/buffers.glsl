layout(rgba32f, binding = 0) uniform image2D outputTex;
layout(rgba32f, binding = 1) uniform image2D accumulatorTex;

layout(std140, binding = 0) uniform Uniforms
{
	mat4x4 u_camView;
	vec3 u_camWorldPos;
	float u_FOV;
	uint u_nSphereLights;
	uint u_nSpheres;
	uint u_nModels;
	uint u_frame;
};

layout(std430, binding = 1) buffer WorkGroupsCount
{
	uint x, y, z;
};

layout(std430, binding = 2) buffer Atomics
{
	uint extendWorkGroup;
	uint shadeWorkGroup;
	uint connectWorkGroup;
	uint extendThreadCounter;
	uint shadeThreadCounter;
	uint connectThreadCounter;
} Atomic;

layout(std430, binding = 3) buffer ExtendBuffer
{
	Ray extendRay[MAX_WIDTH * MAX_HEIGHT * 2];
} ExtQueue;

layout(std140, binding = 4) uniform Swap 
{
	uint in_offset;
	uint out_offset;
};

layout(std430, binding = 5) buffer ShadowBuffer
{
	Ray shadowRay[MAX_WIDTH * MAX_HEIGHT];
} ShadowQueue;

layout(std430, binding = 6) buffer HitInfo
{
	vec3 point[MAX_WIDTH * MAX_HEIGHT];
	vec3 lastPoint[MAX_WIDTH * MAX_HEIGHT];
	vec3 N[MAX_WIDTH * MAX_HEIGHT];
	vec3 V[MAX_WIDTH * MAX_HEIGHT];
	float t[MAX_WIDTH * MAX_HEIGHT];
	uint matid[MAX_WIDTH * MAX_HEIGHT];
	bool emitter[MAX_WIDTH * MAX_HEIGHT];
} Intersection;

layout(std430, binding = 7) buffer PathStates
{
	vec3 throughput[MAX_WIDTH * MAX_HEIGHT];
	vec3 radiance[MAX_WIDTH * MAX_HEIGHT];
	float mediumIOR[MAX_WIDTH * MAX_HEIGHT];
	LightSampleRec lightSampleRec[MAX_WIDTH * MAX_HEIGHT];
} Path;

layout(std430, binding = 8) buffer SceneHierarchy
{
	Material material[MAX_MATERIALS];
	Sphere sphere[MAX_SPHERES];
	SphereLight sphereLight[MAX_SPHERE_LIGHTS];
	Model models[MAX_MODELS];
} Scene;
