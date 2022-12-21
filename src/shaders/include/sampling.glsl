vec3 SampleHemisphere(in vec2 Xi) 
{
	float phi = 2.0 * PI * Xi.x;
	float sinTheta = sqrt(1.0 - Xi.y*Xi.y);

	return vec3(cos(phi) * sinTheta, Xi.y, sin(phi) * sinTheta);
}

vec3 SampleCosineWeightedReflection(in vec2 Xi) 
{
	float r = sqrt(Xi.x);
	float theta = TWO_PI * Xi.y;
	float x = r * cos(theta);
	float z = r * sin(theta);

	return vec3(x, sqrt(1.0 - Xi.x), z);
}

vec3 ImportanceSampleGTR1(in vec2 Xi, in float a) 
{
	float phi = Xi.x * TWO_PI;
	float cosTheta = sqrt((1.0 - pow(a*a, 1.0 - Xi.y)) / (1.0 - a*a));
    float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);

	return vec3(cos(phi) * sinTheta, cosTheta, sinTheta * sin(phi));
}

vec3 ImportanceSampleGGX(in vec2 Xi, in float a)
{
    float phi = Xi.x * TWO_PI;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);

	return vec3(cos(phi) * sinTheta, cosTheta, sinTheta * sin(phi));
}

LightSample SampleSphereLight(in SphereLight light, in vec3 surfacePos)
{
	vec2 Xi = Rand2();

	vec3 sphereCenterToSurface = normalize(surfacePos - light.worldPos);
	vec3 sampleDir = SampleHemisphere(Xi);
	vec3 sampleDirToWorld = ToWorldSpace(sampleDir, sphereCenterToSurface);

	vec3 lightSurfacePos = light.worldPos + sampleDirToWorld * light.radius;
	vec3 normal = normalize(lightSurfacePos - light.worldPos);
	
	vec3 lightDir = lightSurfacePos - surfacePos;
	float dist = distance(lightSurfacePos, surfacePos);

	LightSample ls;
	ls.lightDir = normalize(lightDir);
	ls.emission = light.emittance;
	ls.pdf = (dist * dist) / (light.area * abs(dot(normal, lightDir)));
	ls.dist = dist;

	return ls;
}

float BalancedHeuristic(in float pdf1, in float pdf2)
{
	return pdf1 / (pdf1 + pdf2);
}

float PowerHeuristic(in float pdf1, in float pdf2)
{
	return pdf1 * pdf1 / (pdf1 * pdf1 + pdf2 * pdf2);
}

vec3 EmitterSample(in uint pathid, in uint depth)
{
	LightSampleRec ls = Path.lightSampleRec[pathid];

	vec3 Le;
	if(depth == 0)
		Le = ls.emission;
	else
		Le = PowerHeuristic(ls.bsdfPdf, ls.lightPdf) * ls.emission;
		
	return Le;
}