#version 430 core
#include "include/globals.glsl"
#include "include/buffers.glsl"
#include "include/intersect.glsl"

layout(local_size_x = MIN_WORK_GROUP_INVOCATION_X) in;

bool AnyHit(in Ray r, in float maxDist)
{
	float t;

	// Spheres
	for(uint i = 0; i < u_nSpheres; ++i)
	{
		Sphere s = Scene.sphere[i];

		t = IntersectSphere(s, r);
		float dist = distance(r.origin, r.origin + r.dir*t);

		if(dist < maxDist)
			return true;
	}

	// BVH
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

					t = IntersectTriangle(triangle, r);
					float dist = distance(r.origin, r.origin + r.dir*t);

					if(dist < maxDist)
						return true;
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

	return false;
}

void main()
{
	uint tid = gl_GlobalInvocationID.x;

	if(tid >= Atomic.connectThreadCounter)
		return;

	Ray shadowRay = ShadowQueue.shadowRay[tid];
	float dist = Path.lightSampleRec[shadowRay.pathid].dist;
	
	if(AnyHit(shadowRay, dist))
		Path.lightSampleRec[shadowRay.pathid].bsdfEval = vec3(0.0f, 0.0f, 0.0f);
}