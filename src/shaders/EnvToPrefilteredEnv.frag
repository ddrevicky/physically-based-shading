#version 330 core

uniform samplerCube uHDREnvironmentCubemap;
uniform float uRoughness;

in vec3 vsLocalPosition;
out vec4 fragColor;

#define PI 3.14159265359

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

// Run biased Monte-Carlo integration with importance sampling
void main()
{
	vec3 N = normalize(vsLocalPosition);
	vec3 R = N;
	vec3 V = R;

	vec3 prefilteredColor = vec3(0);
	float totalWeight = 0.0;
	const int numSamples = 1024;
	for(int i = 0; i < numSamples; i++)
	{
		vec2 Xi = Hammersley(uint(i), uint(numSamples));
		vec3 H = ImportanceSampleGGX(Xi, uRoughness, N);				// Randomly (with importance sampling) generated half-vector
		vec3 L = normalize(2 * dot(V, H) * H - V);						// To-light vector (view vector reflected about the sampled half-vector).
		float NdotL = clamp(dot(N, L), 0, 1);
		if(NdotL > 0)
		{
			prefilteredColor += texture(uHDREnvironmentCubemap, L).rgb * NdotL;		// Weighed by NdotL since it looks better (see Epic notes)
			totalWeight += NdotL;
		}
	}
	vec3 irradiance = prefilteredColor / totalWeight;
	fragColor = vec4(irradiance, 1.0);
}