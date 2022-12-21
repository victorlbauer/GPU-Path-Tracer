#version 430 core
#include "include/globals.glsl"
#include "include/buffers.glsl"

layout(local_size_x = MIN_WORK_GROUP_INVOCATION_X) in;

uniform bool u_resetAccumulator;

void ReinhardToneMapping(inout vec4 pixelColor)
{
	pixelColor /= pixelColor.w;
	pixelColor = sqrt(pixelColor / (pixelColor + vec4(1.0)));
}

void main()
{
	uvec2 dims = imageSize(outputTex);
	uint tid = gl_GlobalInvocationID.x;

	if(tid >= dims.x * dims.y)
		return;

	uint x = uint(tid % dims.x);
	uint y = uint(tid / dims.x);
	ivec2 pixelCoords = ivec2(x, y);
	
	vec4 pixelColor;
	if(u_resetAccumulator)
		pixelColor = vec4(Path.radiance[tid], 1.0);
	else
		pixelColor = imageLoad(accumulatorTex, pixelCoords) + vec4(Path.radiance[tid], 1.0);
	
	// TODO: Refactor - Hacky way to deal with NaNs and Infs
	highp bvec3 nan = isnan(Path.radiance[tid]);
	highp bvec3 inf = isinf(Path.radiance[tid]);
	if(nan.x || nan.y || nan.z || inf.x || inf.y || inf.z)
		pixelColor = vec4(BLACK, 1.0);
	
	imageStore(accumulatorTex, pixelCoords, pixelColor);
	
	ReinhardToneMapping(pixelColor);
	imageStore(outputTex, pixelCoords, pixelColor);
}