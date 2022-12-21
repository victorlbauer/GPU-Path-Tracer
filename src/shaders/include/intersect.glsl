void FetchSphereData(in Sphere s, in float t, in Ray r, inout Hit hit)
{
	hit.t = t;
	hit.point = r.origin + r.dir * t;
	hit.N = normalize(hit.point - s.worldPos);
	hit.matid = s.matid;
}

void FetchSphereLightData(in SphereLight sl, in float t, in Ray r, inout Hit hit)
{
	hit.t = t;
	hit.point = r.origin + r.dir * t;
	hit.N = normalize(hit.point - sl.worldPos);
	hit.emitter = true;

	float cosTheta = dot(hit.V, normalize(r.origin - hit.point));
	float dist = distance(hit.point, r.origin);
	Path.lightSampleRec[r.pathid].lightPdf = dist * dist / (sl.area * cosTheta);
	Path.lightSampleRec[r.pathid].emission = sl.emittance;
}

void FetchTriangleData(in Triangle triangle, in uint matid, in float t, in Ray r, inout Hit hit)
{		
	vec3 v0v1 = triangle.vert[1].pos - triangle.vert[0].pos;
	vec3 v0v2 = triangle.vert[2].pos - triangle.vert[0].pos;

	vec3 pvec	 = cross(r.dir, v0v2);
	float det	 = dot(v0v1, pvec);
	float invDet = 1.0 / det;
	vec3 tvec	 = r.origin - triangle.vert[0].pos;
	vec3 qvec	 = cross(tvec, v0v1);

	float u = dot(tvec, pvec) * invDet;
	float v = dot(r.dir, qvec) * invDet;

	hit.t = t;
	hit.point = r.origin + r.dir * t;
	hit.N = normalize(u * triangle.vert[1].normal + v * triangle.vert[2].normal + (1.0 - u - v) * triangle.vert[0].normal);
	//hit.N = normalize(cross(v0v1, v0v2));
	hit.matid = matid;
}

float IntersectSphere(in Sphere s, in Ray r) 
{
	vec3 oc = r.origin - s.worldPos;
	float a = dot(r.dir, r.dir);
	float b = 2.0 * dot(oc, r.dir);
	float c = dot(oc, oc) - s.radius * s.radius;
	float discriminant = b*b - 4*a*c;

	// No intersection
	if(discriminant < 0) 
		return INFINITY;

	float det = sqrt(discriminant);

	// First solution
	float t0 = (-b - det) / (2.0 * a);
	if(t0 > EPSILON) 
		return t0;

	// Inside sphere
	float t1 = (-b + det) / (2.0 * a);
	if(t1 > EPSILON)
		return t1;

	return INFINITY;
}

bool HitSphere(in Sphere s, in Ray r) 
{
	vec3 oc = r.origin - s.worldPos;
	float a = dot(r.dir, r.dir);
	float b = 2.0 * dot(oc, r.dir);
	float c = dot(oc, oc) - s.radius * s.radius;
	float discriminant = b*b - 4*a*c;

	// No intersection
	if(discriminant < 0) 
		return false;

	float det = sqrt(discriminant);

	// First solution
	float t0 = (-b - det) / (2.0 * a);
	if(t0 > EPSILON) 
		return true;

	// Inside sphere
	float t1 = (-b + det) / (2.0 * a);
	if(t1 > EPSILON) 
		return true;

	return false;
}

float IntersectTriangle(in Triangle triangle, in Ray r)
{
	vec3 v0v1 = triangle.vert[1].pos - triangle.vert[0].pos;
	vec3 v0v2 = triangle.vert[2].pos - triangle.vert[0].pos;

	vec3 pvec = cross(r.dir, v0v2);
	float det = dot(v0v1, pvec);

	// if culling
	//if(det < EPSILON)
	//	return INFINITY;

	if(abs(det) < EPSILON)
		return INFINITY;

	float invDet = 1.0 / det;
	vec3 tvec = r.origin - triangle.vert[0].pos;
	float u = dot(tvec, pvec) * invDet;
	if(u < 0.0 || u > 1.0)
		return INFINITY;

	vec3 qvec = cross(tvec, v0v1);
	float v = dot(r.dir, qvec) * invDet;
	if(v < 0.0 || u + v > 1.0) 
		return INFINITY;

	float t = dot(v0v2, qvec) * invDet;
	if(t < 0.0) 
		return INFINITY;

	return t;
}

bool HitTriangle(in Triangle triangle, in Ray r)
{
	vec3 v0v1 = triangle.vert[1].pos - triangle.vert[0].pos;
	vec3 v0v2 = triangle.vert[2].pos - triangle.vert[0].pos;

	vec3 pvec = cross(r.dir, v0v2);
	float det = dot(v0v1, pvec);

	if(abs(det) < EPSILON)
		return false;

	float invDet = 1.0 / det;
	vec3 tvec = r.origin - triangle.vert[0].pos;
	float u = dot(tvec, pvec) * invDet;
	if(u < 0.0 || u > 1.0)
		return false;

	vec3 qvec = cross(tvec, v0v1);
	float v = dot(r.dir, qvec) * invDet;
	if(v < 0.0 || u + v > 1.0) 
		return false;

	float t = dot(v0v2, qvec) * invDet;
	if(t < 0.0) 
		return false;

	return true;
}

float IntersectAABB(in BVHNode node, in vec3 invDir, in Ray r)
{
	vec3 near = (node.boundMin - r.origin) * invDir;
	vec3 far = (node.boundMax - r.origin) * invDir;
	
	vec3 tmin = min(far, near);
	vec3 tmax = max(far, near);
	
	float tNear = max(tmin.x, max(tmin.y, tmin.z));
	float tFar = min(tmax.x, min(tmax.y, tmax.z));

	return (tFar >= tNear) ? (tNear > 0.0 ? tNear : tFar) : -1.0;
}