// Renderer settings
#define MIN_WORK_GROUP_INVOCATION_X 1024
#define MAX_WIDTH	 1920
#define MAX_HEIGHT	 1080
#define MAX_DEPTH	 8
#define RR_MAX_DEPTH 4

// Scene buffer specifics
#define MAX_MATERIALS		64
#define MAX_SPHERE_LIGHTS	16
#define MAX_SPHERES			64
#define MAX_MODELS			8
#define MAX_TRIANGLES		100000
#define MAX_NODES			100000

// Utils
#define INFINITY   1000000
#define EPSILON    0.00001
#define PI		   3.14159265359
#define TWO_PI	   6.28318530718
#define INV_PI	   1.0 / PI

// Colors
#define WHITE vec3(1.0, 1.0, 1.0)
#define BLACK vec3(0.0, 0.0, 0.0)
#define	RED	  vec3(1.0, 0.0, 0.0)
#define GREEN vec3(0.0, 1.0, 0.0)
#define BLUE  vec3(0.0, 0.0, 1.0)

#define MAX_VERTS 50000
#define MAX_IDX	  100000

struct LightSampleRec
{
	vec3  bsdfEval;
	float bsdfPdf;
	vec3  emission;
	float lightPdf;
	vec3 _pad;
	float dist;
};

struct LightSample
{
	vec3 lightDir;
	float dist;
	vec3 emission;
	float pdf;
};

struct Ray 
{
	vec3 origin;
	vec3 dir;
	uint pathid;
};

struct Hit
{
	vec3 point;
	float t;
	vec3 lastPoint;
	uint matid;
	vec3 N;
	bool emitter;
	vec3 V;
};

struct Sphere 
{
	vec3 worldPos;
	float _pad;
	float radius;
	uint matid;
};

struct SphereLight 
{
	vec3 emittance;
	float radius;
	vec3 worldPos;
	float area;
};

struct Vertex
{
	vec3 pos;
	vec3 normal;
};

struct Triangle
{
	Vertex vert[3];
};

struct BVHNode
{
	vec3 boundMin;
	int secondChildOffset;
	vec3 boundMax;
	int nPrimitives;
};

struct Model
{
	Triangle triangles[MAX_TRIANGLES];
	BVHNode bvhnodes[MAX_NODES];
	uint matid;
	uint _pad[3];
};

struct Material 
{
	vec3  baseColor;
	float roughness;
	vec3  density;
	float metalness;
	vec3  emission;
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

// Internal RNG state
uvec4 seed;