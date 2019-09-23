#version 330 core

// Lights
struct PointLight 
{
    vec3 position;

	vec3 ambient;					
    vec3 diffuse;					
    vec3 specular;					
};

# define NUM_POINT_LIGHTS 4
uniform PointLight uPointLights[NUM_POINT_LIGHTS];
uniform vec3 uCameraPosWorld;

// Material
uniform vec3 uAlbedo;
uniform float uMetalness;
uniform float uRoughness;
uniform float uAO;

// Texture material
uniform sampler2D uTexAlbedo;
uniform sampler2D uTexMetalness;
uniform sampler2D uTexRoughness;
uniform sampler2D uTexNormal;

// Environment
uniform sampler2D uTexIntegratedBRDF;
uniform samplerCube uCubeIrradiance;
uniform samplerCube uCubePrefilteredEnvMap;

in VS_OUT 
{
	vec3 normal;
	vec2 texCoords;
	vec3 posWorld;
	vec4 posLightSpace;
} fs_in;

out vec4 fragColor;

const float PI = 3.1415926535897932384626433832795f;
const float gamma = 2.2;

// Schlick approximation to the Fresnel equation
vec3 F_Schlick(vec3 V, vec3 H, vec3 F0)
{
	float VdotH = max(dot(V, H), 0);
	return F0 + (1 - F0) * pow((1 - VdotH), 5);
}

vec3 F_SchlickRoughness(vec3 V, vec3 H, vec3 F0, float roughness)
{
	float VdotH = max(dot(V, H), 0);
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - VdotH, 5.0);
}

float GSchlickGGX(vec3 N, vec3 V, float k)
{
	float NdotV = max(dot(N, V), 0);
	float nom = NdotV;
	float denom = NdotV * (1 - k) + k; 
	return nom / denom;
}

// Smith's method using Schlick-GGX (combination of the GGX and Schlick-Beckmann approximation)
float G_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float k = (roughness + 1) * (roughness + 1) / 8;
	return GSchlickGGX(N, L, k) * GSchlickGGX(N, V, k);
}

// NOTE: a(lpha) = roughness^2
// Trowbridge-Reitz GGX NDF (Normal Distribution Function)
float N_GGXTR(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0);

	float nom = a2;
	float denom = (NdotH * NdotH) * (a2 - 1) + 1;
	denom = PI * denom * denom;
	return nom / denom;
}

void main() 
{
#if 0	// Use textures
	vec3 albedo = texture(uTexAlbedo, fs_in.texCoords).rgb;
	albedo = pow(albedo, vec3(gamma));
	float roughness = texture(uTexRoughness, fs_in.texCoords).r;
	float metalness = texture(uTexMetalness, fs_in.texCoords).r;
	float AO = 1.0;
#else	// Use uniform value
	vec3 albedo = uAlbedo;
	float roughness = uRoughness;
	float metalness = uMetalness;
	float AO = uAO;
#endif

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metalness);

	vec3 N = normalize(fs_in.normal);
	vec3 V = normalize(uCameraPosWorld - fs_in.posWorld);	// Wo
	vec3 p = fs_in.posWorld;
	float VdotN = max(dot(V, N), 0);

	/* Direct lighting */
	// Reflectance integral
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		PointLight pointLight = uPointLights[i];

		vec3 lightColor = vec3(25.0, 25.0, 25.0);
		float dist = length(p - pointLight.position);
		float attenuation = 1.0 / (dist * dist);

		// Incoming Radiance
		vec3 Li = attenuation * lightColor;
		
		vec3 L = normalize(pointLight.position - p);
		vec3 H = normalize(L + V);
		
		// BRDF
		// Lambert diffuse
		vec3 fLambert = albedo / PI;

		// Cook-Torrance Specular
		float D = N_GGXTR(N, H, roughness);
		vec3 F = F_Schlick(V, H, F0);							
		float G = G_Smith(N, V, L, roughness);

		float LdotN = max(dot(N, L), 0);
		vec3 fCookTorrance = (D * F * G) / (4 * VdotN * LdotN + 0.001);

		vec3 kS = F;
		vec3 kD = 1.0 - kS;

		vec3 BRDF = kD * fLambert + fCookTorrance;									// Don't multiply by kS (it's already accounted for in fCookTorrance)
		Lo += BRDF * Li * LdotN;
	}
	vec3 directLight = Lo;

	/* Indirect (ambient) lighting */
	//-------------------------------------------------------------------------
#if 1	// Use environment lighting
	// Indirect lighting consists like direct lighting from the diffuse and specular part (which some up to 1)
	// We can approximate the portion of specular vs diffuse using the Schlick's approximation of Fresnel
	vec3 F = F_SchlickRoughness(V, N, F0, roughness);		
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metalness;

	// Indirect diffuse
	vec3 diffuseIrradiance = texture(uCubeIrradiance, N).rgb;
	vec3 indirectDiffuse = diffuseIrradiance * albedo;
	
	// Indirect specular
	vec3 R = 2 * dot(V, N) * N - V;
	const float MAX_MIP_LEVEL = 4.5;	// Should be 4.9, but buggy at older devices
	float mipLevel = roughness * MAX_MIP_LEVEL;
	vec3 prefilteredColor = textureLod(uCubePrefilteredEnvMap, R, mipLevel).rgb;

	vec2 envBRDF = texture(uTexIntegratedBRDF, vec2(VdotN, roughness)).rg;		// Get F scale and bias from the LUT
	F = F * envBRDF.x + envBRDF.y;
	vec3 indirectSpecular = prefilteredColor * F;

	vec3 indirectLight = (kD * indirectDiffuse + indirectSpecular) * AO;			// kS = F is already accounted for
#else // Don't use environment lighting
	vec3 indirectLight = vec3(0.13) * albedo * AO;
	//-------------------------------------------------------------------------
#endif
	/* Final color */
    vec3 color = directLight + indirectLight;

    color = color / (color + vec3(1.0));	// HDR
	color = pow(color, vec3(1.0 / gamma));	// Gamma correction
	fragColor = vec4(color, 1.0);
}
