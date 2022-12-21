#version 430 core
#include "include/globals.glsl"
#include "include/buffers.glsl"
#include "include/intersect.glsl"

layout(local_size_x = MIN_WORK_GROUP_INVOCATION_X) in;

Hit ClosestHit(in Ray r)
{
	Hit hit;
	hit.point	  = vec3(INFINITY);
	hit.N		  = vec3(0.0);
	hit.t		  = INFINITY;
	hit.matid	  = 0;
	hit.emitter	  = false;

	hit.lastPoint = r.origin;
	hit.V = normalize(-r.dir);

	float tNear = INFINITY;
	float t;

	// Check for collision against spheres
	int sphereId = -1;
	for(int i = 0; i < u_nSpheres; ++i)
	{
		Sphere s = Scene.sphere[i];
		if((t = IntersectSphere(s, r)) != INFINITY && t < tNear)
		{
			tNear = t;
			sphereId = i;
		}
	}

	if(sphereId > -1)
		FetchSphereData(Scene.sphere[sphereId], tNear, r, hit);

	// BVH traversal
	int triangleId = -1;
	int modelId = -1;
	vec3 invDir = 1.0 / r.dir;

	int stack[64];
	int ptr = 0;

	for(int i = 0; i < u_nModels; ++i) 
	{
		// Null node
		stack[ptr++] = -1;

		// Stack the root node
		stack[ptr] = 0;

		do
		{
			int idx = stack[ptr--];
			BVHNode node = Scene.models[i].bvhnodes[idx];
			
			// Leaf node
			if(node.nPrimitives > 0)
			{
				// Intersect with primitives (triangles, in this case)
				for(int j = 0; j < node.nPrimitives; ++j)
				{
					Triangle triangle = Scene.models[i].triangles[node.secondChildOffset + j];
					if((t = IntersectTriangle(triangle, r)) != INFINITY && t < tNear)
					{
						tNear = t;
						triangleId = node.secondChildOffset + j;
						modelId = i;
					}
				}
			}
			else
			{	
				BVHNode leftChild  = Scene.models[i].bvhnodes[idx + 1];
				BVHNode rightChild = Scene.models[i].bvhnodes[node.secondChildOffset];

				float tLeft	 = IntersectAABB(leftChild, invDir, r);
				float tRight = IntersectAABB(rightChild, invDir, r);

				if(tLeft > 0.0 && tRight > 0.0)
				{
					if(tLeft < tRight)
					{
						stack[++ptr] = node.secondChildOffset;
						stack[++ptr] = idx + 1;
					}
					else
					{
						stack[++ptr] = idx + 1;
						stack[++ptr] = node.secondChildOffset;
					}
				}
				else if(tLeft > 0.0)
					stack[++ptr] = idx + 1;
				else if(tRight > 0.0)
					stack[++ptr] = node.secondChildOffset;
			}
		} while(ptr > 0);
	}

	if(triangleId > -1)
		FetchTriangleData(Scene.models[modelId].triangles[triangleId], Scene.models[modelId].matid, tNear, r, hit);

	// Check for collisions against sphere lights
	int lightId = -1;
	for(int i = 0; i < u_nSphereLights; ++i)
	{
		SphereLight sl = Scene.sphereLight[i];

		Sphere s;
		s.worldPos = sl.worldPos;
		s.radius = sl.radius;

		if((t = IntersectSphere(s, r)) != INFINITY && t < tNear)
		{
			tNear = t;
			lightId = i;
		}
	}
	
	if(lightId > -1)
		FetchSphereLightData(Scene.sphereLight[lightId], tNear, r, hit);

	return hit;
};

void main()
{
	uint tid = gl_GlobalInvocationID.x;
	
	if(tid >= Atomic.extendThreadCounter)
		return;

	uint nthreads = atomicAdd(Atomic.shadeThreadCounter, 1);
	if(nthreads % MIN_WORK_GROUP_INVOCATION_X == 0)
		atomicAdd(Atomic.shadeWorkGroup, 1);

	Ray extendRay = ExtQueue.extendRay[in_offset + tid];
	Hit hit = ClosestHit(extendRay);

	if(hit.t == INFINITY)
	{
		Intersection.t[tid] = INFINITY;
		Intersection.V[tid] = hit.V;
		return;
	}
	
	// Enqueue results
	Intersection.point[tid]		= hit.point;
	Intersection.lastPoint[tid]	= hit.lastPoint;
	Intersection.N[tid]			= hit.N;
	Intersection.V[tid]			= hit.V;
	Intersection.t[tid]			= hit.t;
	Intersection.matid[tid]		= hit.matid;
	Intersection.emitter[tid]	= hit.emitter;
}