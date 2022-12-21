#version 430 core
#include "include/globals.glsl"
#include "include/utils.glsl"
#include "include/buffers.glsl"
#include "include/sampling.glsl"
#include "include/principled.glsl"

layout(local_size_x = MIN_WORK_GROUP_INVOCATION_X) in;

uniform sampler2D u_HDRI;
uniform uint u_depth;

void GenerateExtendRay(in vec3 N, in vec3 L, in vec3 rayOrigin, in uint pathid, in uint nthreads)
{
	Ray r;
	r.origin = rayOrigin + N * EPSILON;
	r.dir = L;
	r.pathid = pathid;
	
	if(nthreads % MIN_WORK_GROUP_INVOCATION_X == 0)
		atomicAdd(Atomic.extendWorkGroup, 1);

	ExtQueue.extendRay[out_offset + nthreads] = r;
}

void GenerateShadowRay(in vec3 N, in vec3 L, in vec3 rayOrigin, in uint pathid, in uint nthreads)
{
	Ray sr;
	sr.origin = rayOrigin + N * EPSILON;
	sr.dir = L;
	sr.pathid = pathid;
	
	if(nthreads % MIN_WORK_GROUP_INVOCATION_X == 0)
		atomicAdd(Atomic.connectWorkGroup, 1);

	ShadowQueue.shadowRay[nthreads] = sr;
}

bool PathTerminated(in uint pathid)
{
	uint tid = gl_GlobalInvocationID.x;

	// Hit background
	if(Intersection.t[tid] == INFINITY)
	{
		// TODO: MIS Env Map
		float exposure = 3.0;
		vec3 dir = normalize(-Intersection.V[tid]);
		vec2 uv = vec2((PI + atan(dir.z, dir.x)) * (1.0 / (TWO_PI)), acos(-dir.y) * (1.0 / PI));
		Path.radiance[pathid] += Path.throughput[pathid] * exposure * texture(u_HDRI, uv).xyz;
		//Path.radiance[pathid] += Path.throughput[pathid] * vec3(0.0);
		return true;
	}
	// Hit a light
	else if(Intersection.emitter[tid] == true)
	{
		Path.radiance[pathid] += EmitterSample(pathid, u_depth) * Path.throughput[pathid];
		return true;
	}
	// Russian roullete elimination
	else if(u_depth >= RR_MAX_DEPTH)
	{
		float Xi = Rand();
		float p = max(Path.throughput[pathid].x, max(Path.throughput[pathid].y, Path.throughput[pathid].z));
	
		if(Xi > p)
			return true;
	
		Path.throughput[pathid] /= p;
	}
	// Reached max depth
	else if(u_depth >= MAX_DEPTH)
	{
		Path.throughput[pathid] = BLACK;
		return true;
	}
	
	return false;
}

void main()
{
	uint tid = gl_GlobalInvocationID.x;

	if(tid >= Atomic.shadeThreadCounter)
		return;

	SetSeed(gl_GlobalInvocationID.xy, u_frame);

	uint pathid = ExtQueue.extendRay[in_offset + tid].pathid;

	// Direct lighting contribution
	Path.radiance[pathid] += Path.lightSampleRec[pathid].bsdfEval * Path.throughput[pathid];
	
	if(PathTerminated(pathid))
		return;
	
	uint matid = Intersection.matid[tid];
	Material mat = Scene.material[matid];

	vec3 L, H;
	vec3 N = Intersection.N[tid];
	vec3 V = Intersection.V[tid];
	
	vec3 bsdf = BLACK;
	float bsdfPdf = 1.0;

	// Indirect lighting evaluation
	PrincipledSample(tid, pathid, mat, N, V, L, H, bsdf, bsdfPdf);
	Path.throughput[pathid] *= abs(dot(N, L)) * bsdf / bsdfPdf;

	// Indirect lighting BSDF pdf for the next iteration MIS
	Path.lightSampleRec[pathid].bsdfPdf = bsdfPdf;
	
	// Extend this path for the next iteration
	uint nthreads = atomicAdd(Atomic.extendThreadCounter, 1);
	GenerateExtendRay(N, L, Intersection.point[tid], pathid, nthreads);

	// Generate light sample (TODO: change to random light)
	LightSample ls = SampleSphereLight(Scene.sphereLight[0], Intersection.point[tid]);
	
	PrincipledEval(tid, pathid, mat, N, V, ls.lightDir, bsdf, bsdfPdf);
	Path.lightSampleRec[pathid].bsdfEval = PowerHeuristic(ls.pdf, bsdfPdf) * ls.emission * abs(dot(N, ls.lightDir)) * bsdf / ls.pdf;
	Path.lightSampleRec[pathid].dist = ls.dist - EPSILON;
	
	// Generate shadow ray
	atomicAdd(Atomic.connectThreadCounter, 1);
	GenerateShadowRay(N, ls.lightDir, Intersection.point[tid], pathid, nthreads);
}