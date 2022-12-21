// Generalized Trowbridge-Reitz 1 as described by Brent Burley's notes
float GTR1(in float ndoth, in float a)
{
	float a2 = a * a;
	float denom = (1.0 + (a2 - 1.0) * ndoth * ndoth);
	return (a2 - 1.0) / (PI * log2(a2) * denom);
}

// Trowbridge-Reitz (GGX)
float GGXDistribution(in float ndoth, in float a)
{
	float a2 = a * a;
	float denom = 1.0 + (a2 - 1.0) * ndoth * ndoth;
	return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(in float ndotv, in float k) 
{
	return ndotv / (ndotv * (1.0 - k) + k);
}

float SmithGGX(in float cosTheta, in float a) 
{
	return 1.0 / (cosTheta + sqrt(a*a + cosTheta*cosTheta - a*a * cosTheta*cosTheta));
}

float GeometrySmith(in vec3 N, in vec3 V, in vec3 L, in float k) 
{
	float ndotv = abs(dot(N, V));
	float ndotl = abs(dot(N, L));
	float ggx1 = GeometrySchlickGGX(ndotv, k);
	float ggx2 = GeometrySchlickGGX(ndotl, k);
	return ggx1 * ggx2;
}

float DielectricFresnel(in float cosTheta, in float IOR) 
{
	float sinTheta = IOR * IOR * (1.0 - cosTheta * cosTheta);
	
	// Total internal reflection
	if(sinTheta > 1.0) 
		return 1.0;

	float cosThetaT = sqrt(max(1.0 - sinTheta, 0.0));
	float rs = (IOR * cosThetaT - cosTheta) / (IOR * cosThetaT + cosTheta);
	float rp = (IOR * cosTheta - cosThetaT) / (IOR * cosTheta + cosThetaT);

	return 0.5 * (rs * rs + rp * rp);
}

vec3 FresnelSchlick(in float costheta, in vec3 F0) 
{
	return F0 + (1.0 - F0) * pow(1.0 - costheta, 5.0);
}

float FresnelSchlick(in float costheta, in float F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - costheta, 5.0);
}

vec3 EvalDielectricReflection(in Material mat, in vec3 N, in vec3 V, in vec3 L, in vec3 H, inout float pdf) 
{
	if(dot(N, L) < EPSILON)
		return BLACK;

	float D = GGXDistribution(dot(N, H), mat.roughness);
	float G = SmithGGX(abs(dot(N, L)), mat.roughness) * SmithGGX(abs(dot(N, V)), mat.roughness);
	float F = FresnelSchlick(abs(dot(V, H)), mat.specular);
	
	pdf = D * dot(N, H) * F / (4.0 * abs(dot(V, H)));
	return mat.baseColor * F * D * G;
}

vec3 EvalDielectricRefraction(in Material mat, in vec3 N, in vec3 V, in vec3 L, in vec3 H, inout float pdf, in float eta) 
{
	if(dot(N, L) > EPSILON)
		return BLACK;

	float D = GGXDistribution(dot(N, H), mat.roughness);
	float G = SmithGGX(abs(dot(N, L)), mat.roughness) * SmithGGX(abs(dot(N, V)), mat.roughness);
	float F = DielectricFresnel(abs(dot(V, H)), eta);
	
	float denomSqrt = dot(L, H) + dot(V, H) * eta;
	pdf = D * dot(N, H) * (1.0 - F) * abs(dot(L, H)) / (denomSqrt * denomSqrt);
	
	return mat.baseColor * (1.0 - F) * D * G * abs(dot(V, H)) * abs(dot(L, H)) * 4.0 * eta * eta / (denomSqrt * denomSqrt);
}

vec3 EvalDiffuseReflection(in Material mat, in vec3 N, in vec3 V, in vec3 L, in vec3 H, inout float pdf) 
{
    if(dot(N, L) < EPSILON)
        return BLACK;

	// Disney's retro-reflection factor
	float FL = FresnelSchlick(abs(dot(N, L)), 0.0);
	float FV = FresnelSchlick(abs(dot(N, V)), 0.0);
	float FH = FresnelSchlick(abs(dot(L, H)), 0.0);
	float Fd90 = 0.5 + 2.0 * mat.roughness * abs(dot(L, H)) * abs(dot(L, H));
	float F = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);
	
	// Fake Hanrahan-Krueger Subsurface Scattering
	float Fss90 = abs(dot(L, H)) * abs(dot(L, H)) * mat.roughness;
	float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
	float ss = 1.25 * (Fss * (1.0 / (abs(dot(N, L)) + abs(dot(N, V))) - 0.5) + 0.5);

	// Sheen
	float luminance = dot(vec3(0.3, 0.6, 0.1), mat.baseColor);
	vec3 tint = (luminance > 0.0) ? mat.baseColor / luminance : WHITE;
	vec3 sheenTint = mix(WHITE, tint, mat.sheenTint) * FH * mat.sheen;

	pdf = abs(dot(N, L)) * INV_PI;
	
	return (INV_PI * mat.baseColor * mix(F, ss, mat.subsurface) + sheenTint) * (1.0 - mat.metalness) * (1.0 - mat.transmission);
}

vec3 EvalSpecularReflection(in Material mat, in vec3 N, in vec3 V, in vec3 L, in vec3 H, inout float pdf) 
{
	if(dot(N, L) < EPSILON)
		return BLACK;

	float luminance = dot(vec3(0.3, 0.6, 0.1), mat.baseColor);
	vec3 tint = (luminance > 0.0) ? mat.baseColor / luminance : WHITE;
	vec3 specularTint = mix(mat.specular * mix(WHITE, tint, mat.specularTint), mat.baseColor, mat.metalness);
	
	float ndotl = abs(dot(N, L));
	float ndotv = abs(dot(N, V));
	float ndoth = abs(dot(N, H));
	float hdotv = abs(dot(H, V));

	float k = pow(mat.roughness + 1.0, 2.0) / 8.0;
	float D = GGXDistribution(ndoth, mat.roughness);
	float G = GeometrySmith(N, V, L, k);
	vec3  F = FresnelSchlick(abs(dot(L, H)), mix(specularTint, mat.baseColor, mat.metalness)) * mix(WHITE, tint, mat.specularTint);

	pdf = D * ndoth / (4.0 * hdotv);

	return vec3(D * G * F) / (4.0 * ndotl * ndotv);
}

vec3 EvalClearCoat(in Material mat, in vec3 N, in vec3 V, in vec3 L, in vec3 H, inout float pdf) 
{
	if(dot(N, L) < EPSILON)
		return BLACK;

	float D = GTR1(abs(dot(N, H)), mat.clearCoatRoughness);
	float G = SmithGGX(abs(dot(N, L)), mat.clearCoatRoughness) * SmithGGX(abs(dot(N, V)), mat.clearCoatRoughness);
	vec3  F = FresnelSchlick(abs(dot(L, H)), vec3(0.05));

	pdf = D * abs(dot(N, H)) / (4.0 * abs(dot(V, H)));

	return vec3(0.25 * mat.clearCoat * D * G * F);
}

void PrincipledSample(in uint tid, in uint pathid, in Material mat, inout vec3 N, inout vec3 V, inout vec3 L, inout vec3 H, inout vec3 bsdf, inout float pdf)
{
	bsdf = BLACK;
	pdf = 1.0;

	float p = Rand();
	vec2 Xi = Rand2();

	float transWeight = (1.0 - mat.metalness) * mat.transmission;
	float diffuseWeight = 0.5 * (1.0 - mat.metalness);
	float primarySpecRatio = 1.0 / (1.0 + mat.clearCoat);

	// Transmission
	if(p < transWeight) 
	{
		bool fromOutside = dot(-V, N) < 0.0;
		float eta = fromOutside ? (Path.mediumIOR[pathid] / mat.IOR) : (mat.IOR / Path.mediumIOR[pathid]);

		vec3 GGX = ImportanceSampleGGX(Xi.xy, mat.roughness);	
		N = fromOutside ? N : -N;
		H = ToWorldSpace(GGX, N);
	
		vec3 R = reflect(-V, H);
		float F = DielectricFresnel(abs(dot(R, H)), eta);
		
		// Reflect
		if(Rand() < F) 
		{
			L = normalize(R);
			bsdf = EvalDielectricReflection(mat, N, V, L, H, pdf);
		}
		// Refract
		else 
		{
			L = normalize(refract(-V, H, eta));
			bsdf = EvalDielectricRefraction(mat, N, V, L, H, pdf, eta);
			N = -N;
		}
	
		// Absorption
		if(!fromOutside)
		{
			float dist = distance(Intersection.point[tid], Intersection.lastPoint[tid]);
			bsdf *= exp(-dist * mat.density);
		}
	
		bsdf *= transWeight;
		pdf *= transWeight;
	}
	else 
	{
		// Diffuse
		if(p < diffuseWeight)
		{
			vec3 CosineDir = SampleCosineWeightedReflection(Xi);
			L = ToWorldSpace(CosineDir, N);
			H = normalize(V + L);
			bsdf = EvalDiffuseReflection(mat, N, V, L, H, pdf);
			pdf *= diffuseWeight;
		}
		else
		{
			// Specular
			if(p < primarySpecRatio) 
			{
				vec3 GGX = ImportanceSampleGGX(Xi, mat.roughness);
				H = ToWorldSpace(GGX, N);
				L = normalize(reflect(-V, H));
				bsdf = EvalSpecularReflection(mat, N, V, L, H, pdf);
				pdf *= primarySpecRatio * (1.0 - diffuseWeight);
			}
			// Clearcoat
			else 
			{
				vec3 GTR1 = ImportanceSampleGTR1(Xi, mat.clearCoatRoughness);
				H = ToWorldSpace(GTR1, N);
				L = normalize(reflect(-V, H));
				bsdf = EvalClearCoat(mat, N, V, L, H, pdf);
				pdf *= (1.0 - primarySpecRatio) * (1.0 - diffuseWeight);
			}
		}
		bsdf *= (1.0 - transWeight);
		pdf *= (1.0 - transWeight);
	}
}

void PrincipledEval(in uint tid, in uint pathid, in Material mat, in vec3 N, in vec3 V, in vec3 L, inout vec3 bsdf, inout float pdf)
{
	bsdf = BLACK;
	pdf = 1.0;

	vec3 H;
	bool reflected = dot(N, L) > 0.0;
	float eta = reflected ? (mat.IOR / Path.mediumIOR[pathid]) : (Path.mediumIOR[pathid] / mat.IOR);

	if(reflected)
		H = normalize(L + V);
	else
		H = normalize(L + V*eta);

	if(dot(V, H) < 0.0)
		H = -H;

	float transWeight = (1.0 - mat.metalness) * mat.transmission;
	float diffuseWeight = 0.5 * (1.0 - mat.metalness);
	float primarySpecRatio = 1.0 / (1.0 + mat.clearCoat);

	vec3 brdf = BLACK;
	vec3 btdf = BLACK;
	float brdfPdf = 1.0;
	float btdfPdf = 1.0;

	if(transWeight > 0.0)
	{
		if(reflected)
			btdf = EvalDielectricReflection(mat, N, V, L, H, btdfPdf);
		else
		{
			btdf = EvalDielectricRefraction(mat, N, V, L, H, btdfPdf, eta);

			float dist = distance(Intersection.point[tid], Intersection.lastPoint[tid]);
			btdf *= exp(-dist * mat.density);
		}
	}

	float m_pdf;

	if(transWeight < 1.0)
	{
		brdf += EvalDiffuseReflection(mat, N, V, L, H, m_pdf);
		brdfPdf += m_pdf * diffuseWeight;
		
		brdf += EvalSpecularReflection(mat, N, V, L, H, m_pdf);
		brdfPdf += m_pdf * primarySpecRatio * (1.0 - diffuseWeight);
		
		brdf += EvalClearCoat(mat, N, V, L, H, m_pdf);
		brdfPdf += m_pdf * (1.0 - primarySpecRatio) * (1.0 - diffuseWeight);
	}

	bsdf = mix(brdf, btdf, transWeight);
	pdf = mix(brdfPdf, btdfPdf, transWeight);
}