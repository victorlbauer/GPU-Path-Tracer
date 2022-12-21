#version 430 core
#include "include/globals.glsl"
#include "include/buffers.glsl"
#include "include/utils.glsl"

layout(local_size_x = MIN_WORK_GROUP_INVOCATION_X) in;

void StartPathState(in uint pathid)
{
	Path.throughput[pathid]	= vec3(1.0);
	Path.radiance[pathid]	= vec3(0.0);
	Path.mediumIOR[pathid]	= 1.0;

	Path.lightSampleRec[pathid].bsdfEval = vec3(0.0);
	Path.lightSampleRec[pathid].emission = vec3(0.0);
	Path.lightSampleRec[pathid].bsdfPdf  = 0.0;
	Path.lightSampleRec[pathid].lightPdf = 0.0;
	Path.lightSampleRec[pathid].dist = INFINITY;
};

Ray GeneratePrimaryRay(in uint pathid)
{
	uvec2 dims = imageSize(outputTex);
	float aspectRatio = float(dims.x) / float(dims.y);
	float FOV = tan(radians(u_FOV / 2.0));

	vec2 antiAliasing = Rand2();
	float xoffset = antiAliasing.x;
	float yoffset = antiAliasing.y;
	
	uvec2 pixelCoords = uvec2(uint(pathid % dims.x), uint(pathid / dims.x));
	
	float x = (float((pixelCoords.x + xoffset) * 2.0 - dims.x) / dims.x) * aspectRatio * FOV;
	float y = (float((pixelCoords.y + yoffset) * 2.0 - dims.y) / dims.y) * FOV;
	
	vec3 rayOriginToWorld = vec3(vec4(0.0, 0.0, 0.0, 1.0) * u_camView);
	vec3 rayDirToWorld = vec3(vec4(x, y, -1.0, 1.0) * u_camView);
	
	Ray r;
	r.origin = u_camWorldPos;
	r.dir = normalize(rayDirToWorld - rayOriginToWorld);
	r.pathid = pathid;
	
	return r;
}

void main()
{
	uvec2 dims = imageSize(outputTex);
	uint tid = gl_GlobalInvocationID.x;

	if(tid >= dims.x * dims.y) 
		return;

	SetSeed(gl_GlobalInvocationID.xy, u_frame);

	StartPathState(tid);
	ExtQueue.extendRay[in_offset + tid] = GeneratePrimaryRay(tid);

	uint nthreads = atomicAdd(Atomic.extendThreadCounter, 1);
	if(nthreads % MIN_WORK_GROUP_INVOCATION_X == 0)
		atomicAdd(Atomic.extendWorkGroup, 1);
}