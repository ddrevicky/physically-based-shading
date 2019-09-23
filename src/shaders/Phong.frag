#version 330 core

struct DirectionalLight 
{
    vec3 direction;
    vec3 ambient;					// Light's contribution to ambient lighting
    vec3 diffuse;					// Intensity of light diffuse component
    vec3 specular;					// Intensity of light specular component
};

struct PointLight 
{
    vec3 position;

	vec3 ambient;					// Light's contribution to ambient lighting
    vec3 diffuse;					// Intensity of light diffuse component
    vec3 specular;					// Intensity of light specular component

};

struct Material 
{
    vec3 ambient;					// Ambient reflection constant
    vec3 diffuse;					// Diffuse reflection constant
    vec3 specular;					// Specular reflection constant
    float shininess;				// Shininess constant
}; 

uniform DirectionalLight uDirectionalLight;
# define NUM_POINT_LIGHTS 4
uniform PointLight uPointLights[NUM_POINT_LIGHTS];

uniform Material uMaterial;
uniform vec3 uCameraPosWorld;
uniform sampler2D uTexSampler0;		// Shadow map

in VS_OUT 
{
	vec3 normal;
	vec2 texCoords;
	vec3 posWorld;
	vec4 posLightSpace;
} fs_in;

out vec4 fragColor;

vec3 PointLightCalc(PointLight light, vec3 N, vec3 V);
vec3 DirectionalLightCalc(DirectionalLight light, vec4 fragLightSpacePos, vec3 N, vec3 V);
float ShadowCalculation(vec4 posLightSpace, vec3 normal, vec3 lightDir);

void main() 
{
	vec3 color = vec3(0.0f);
	vec3 N = normalize(fs_in.normal);
	vec3 V = normalize(uCameraPosWorld - fs_in.posWorld);

	//color += DirectionalLightCalc(uDirectionalLight, fs_in.posLightSpace, N, V);
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		PointLight pointLight = uPointLights[i];
		color += PointLightCalc(pointLight, N, V);		
	}

	float gamma = 2.2;
	fragColor.rgb = pow(color, vec3(1.0 / gamma));
	fragColor.a = 1.0;
}

vec3 PointLightCalc(PointLight light, vec3 N, vec3 V)
{
		// Point light attenuation
		float dist = length(light.position - fs_in.posWorld);
		float attenuation = 1.0f / (dist * dist); 
		light.ambient *= attenuation;
		light.diffuse*= attenuation;
		light.specular *= attenuation;

		// Ambient
		vec3 ambientLight = uMaterial.ambient * light.ambient;

		// Diffuse
		vec3 I = normalize(light.position - fs_in.posWorld);
		float NIdot = max(dot(I, N), 0.0);
		vec3 diffuseLight = uMaterial.diffuse * NIdot * light.diffuse;

		// Specular
//#define PHONG
#ifdef PHONG
		vec3 R = normalize(reflect(-I, N)); // == normalize(2*dot(N, I)*N - I);
		float specFactor = max(dot(R, V), 0.0);
#else	// BLINN-PHONG
		vec3 H = normalize(V + I);
		float specFactor = max(dot(N, H), 0.0);
#endif
		vec3 specularLight = max(uMaterial.specular * pow(specFactor, uMaterial.shininess) * light.specular, vec3(0)); 

		vec3 color = ambientLight + (diffuseLight + specularLight);
		return color;
}

vec3 DirectionalLightCalc(DirectionalLight light, vec4 fragLightSpacePos, vec3 N, vec3 V)
{
	// Ambient
	vec3 ambientLight = uMaterial.ambient * light.ambient;

	// Diffuse
	vec3 I = -light.direction;
	float NIdot = max(dot(I, N), 0.0);
	vec3 diffuseLight = uMaterial.diffuse * NIdot * light.diffuse;

	// Specular
#ifdef PHONG
	vec3 R = normalize(reflect(-I, N)); // == normalize(2*dot(N, I)*N - I);
	float RVdot = max(dot(R, V), 0.0);
	float specFactor = RVdot;
#else	// BLINN-PHONG
	vec3 H = normalize(V + I);
	float NHdot = max(dot(N, H), 0.0);
	float specFactor = NHdot;
#endif
	vec3 specularLight = uMaterial.specular * pow(specFactor, uMaterial.shininess) * light.specular; 

	float shadow = ShadowCalculation(fragLightSpacePos, N, I);
	shadow = min(shadow, 0.7);
	vec3 color = ambientLight + (diffuseLight + specularLight) * (1.0 - shadow);
	return color;
}

float ShadowCalculation(vec4 posLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 projCoords = posLightSpace.xyz /= posLightSpace.w;
	projCoords = 0.5 * projCoords + 0.5;
	if (projCoords.z > 1.0)		// Fragment is past the far range of the projection
		return 0.0;

	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
	vec2 texelSize = 1.0 / textureSize(uTexSampler0, 0);

	float shadow = 0.0;
#if 1
	int kernelSize = 3;
	for (int i = -kernelSize; i <= kernelSize; ++i)
	{
		for (int j = -kernelSize; j <= kernelSize; ++j)
		{
			float shadowMapDepth = texture(uTexSampler0, projCoords.xy + texelSize * vec2(i, j)).r;
			shadow += currentDepth > shadowMapDepth + bias ? 1.0 : 0.0;
		}
	}
	shadow /= kernelSize * kernelSize;
#else
	vec2 poissonDisk[4] = vec2[]
	(
		vec2( -0.94201624, -0.39906216 ),
		vec2( 0.94558609, -0.76890725 ),
		vec2( -0.094184101, -0.92938870 ),
		vec2( 0.34495938, 0.29387760 )
	);
	for (int i = 0; i < 4; ++i)
	{
		float shadowMapDepth = texture(uTexSampler0, projCoords.xy + texelSize * poissonDisk[i]).r;
		shadow += currentDepth > shadowMapDepth + bias ? 1.0 : 0.0;
	}
#endif
	return shadow;
}
