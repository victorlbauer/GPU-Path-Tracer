vec3 ToWorldSpace(in vec3 localVec, in vec3 N)
{
	vec3 tangent = normalize(abs(N.x) > abs(N.z) ? vec3(-N.y, N.x, 0.0) : vec3(0.0, -N.z, N.y));
	vec3 bitangent = normalize(cross(N, tangent));
	return normalize(tangent * localVec.x + N * localVec.y + bitangent * localVec.z);
}

void SetSeed(in vec2 pixel, in uint frame)
{
	seed = uvec4(pixel, frame, uint(pixel.x) + uint(pixel.y));
}

void PCG4D(inout uvec4 v)
{
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
}

float Rand()
{
    PCG4D(seed); 
	return float(seed.x) / float(0xffffffffu);
}

vec2 Rand2()
{
    PCG4D(seed);
	return seed.xy / float(0xffffffffu);
}

vec3 Rand3()
{
    PCG4D(seed); 
	return seed.xyz / float(0xffffffffu);
}

vec4 Rand4()
{
    PCG4D(seed); 
	return seed.xyzw / float(0xffffffffu);
}

float RadicalInverse(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse(i));
}