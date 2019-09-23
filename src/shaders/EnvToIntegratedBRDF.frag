#version 330 core

in vec2 vsTexCoords;
out vec2 fragColor;

#define PI 3.14159265359

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N);
float G_Smith(vec3 N, vec3 V, vec3 L, float roughness);
float saturate(float x);

// Run biased Monte-Carlo integration with importance sampling
// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
void main()
{
	float NdotV = vsTexCoords.x;	// cosThetaV
	float roughness = vsTexCoords.y;

	vec3 N = vec3(0, 0, 1);
	vec3 V;
	V.x = sqrt(1 - NdotV * NdotV);
	V.y = 0.0;
	V.z = NdotV;
	
	float F0_scale = 0;
	float F0_bias = 0;

	const uint numSamples = 1024u;
	for (uint i = 0u; i < numSamples; ++i)
	{
		vec2 Xi = Hammersley(i, numSamples);
		vec3 H = ImportanceSampleGGX(Xi, roughness, N);					// Randomly (with importance sampling) generated half-vector
		vec3 L = normalize(2 * dot(V, H) * H - V);						// To-light vector (view vector reflected about the sampled half-vector).

		float NdotL = saturate(L.z);
		float NdotH = saturate(H.z);
		float VdotH = saturate(dot(V, H));

		if (NdotL > 0)
		{
			float G = G_Smith(N, V, L, roughness); 
			float G_Vis = G * VdotH / (NdotH * NdotV);	// VdotH acts in place of the NDF here
			float Fc = pow(1 - VdotH, 5);
			
			F0_scale += G_Vis * (1 - Fc);
			F0_bias += G_Vis * Fc;
		}
	}

	fragColor = vec2(F0_scale, F0_bias) / float(numSamples);
}

// Quasi Monte Carlo sequence generation
// Source: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

// Source: http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// Returns the importance sampled half-vector H sampled from the hempisphere around the outgoing direction. The 
// generation is modulated by the roughness parameter so that H has a higher chance of being in the specular lobe.
// Theta is generated using modified cosinus mapping (see http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html)
vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
	float a = roughness * roughness;
	// phi = azimuth , theta = polar angle
	float phi = 2 * PI * Xi.x;
	float cosTheta = sqrt((1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ));
	float sinTheta = sqrt(1 - cosTheta * cosTheta);					// from identity sin^2 + cos^2 = 1

	// Tangent vector on the unit sphere generated from the angles phi, theta
	vec3 H;						
	H.x = sinTheta * cos(phi);
	H.y = sinTheta * sin(phi);
	H.z = cosTheta;
	
	// Construct the world space vectors
	vec3 up = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 tangentX = normalize(cross(up, N));
	vec3 tangentY = normalize(cross(N, tangentX));
	
	// Project the tangent vector to world space
	return tangentX * H.x + tangentY * H.y + N * H.z;
}

float GSchlickGGX(vec3 N, vec3 V, float roughness)
{
	float a = roughness * roughness;
	float k = a / 2.0;

	float NdotV = max(dot(N, V), 0);
	float nom = NdotV;
	float denom = NdotV * (1 - k) + k; 
	return nom / denom;
}

// Smith's method using Schlick-GGX (combination of the GGX and Schlick-Beckmann approximation)
float G_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
	return GSchlickGGX(N, L, roughness) * GSchlickGGX(N, V, roughness);
}

float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
}

