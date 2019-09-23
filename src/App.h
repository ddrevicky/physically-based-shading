#pragma once

#include <map>
#include <string>

#include <glad/glad.h> 
#include <glm/glm.hpp>

#include "Def.h"
#include "Graphics.h"
#include "Camera.h"
#include "UtilMesh.h"
#include "UserInput.h"

struct UserInput;

using glm::vec3;

struct PhongMaterial 
{
    vec3 ambient;		
    vec3 diffuse;		
    vec3 specular;		
    float shininess = 0.0f;
};

struct PBRMaterial
{
	vec3 albedo;
	float metalness = 0.0f;
	float roughness = 0.0f;
	float AO = 0.0f;
};

struct DirectionalLight 
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

enum Shader
{
	Phong,
	PBR,
	TextureDisplay,
	Debug,
	ShadowMap,
	SkyBox,
	EquirectToCubemap,
	EnvToIrradiance,
	EnvToPrefilteredEnv,
	EnvToIntegratedBRDF
};

namespace PBRSamplers
{
	enum TextureSamplers
	{
		Albedo2D,
		Metalness2D,
		Roughness2D,
		Normal2D,
		IntegratedBRDF2D,
		IrradianceMapCube,
		PrefilteredEnvMapCube
	};
}

namespace PhongSamplers
{
	enum TextureSamplers
	{
		Shadowmap2D
	};
}

struct SceneObject
{
	Model model;
	Texture *albedoTexture;
	Texture *metalnessTexture;
	Texture *roughnessTexture;
	Texture *normalTexture;

	unsigned int materialIndex;
	glm::mat4 modelMatrix;

	SceneObject()
	{
		materialIndex = 0;
		modelMatrix = glm::mat4();
	}

	SceneObject(Mesh mesh, Texture *albedoTex, Texture *metalnessTex, Texture *roughnessTex, Texture *normalTex,
				unsigned int materialIndex, glm::mat4 modelMatrix = glm::mat4())
	{
		Graphics::InitModel(&this->model, mesh);
		this->albedoTexture = albedoTex;
		this->metalnessTexture = metalnessTex;
		this->roughnessTexture = roughnessTex;
		this->normalTexture = normalTex;

		this->materialIndex = materialIndex;
		this->modelMatrix = modelMatrix;
	}
};

struct SceneContext
{
	DirectionalLight directionalLight;
	std::vector<PointLight> pointLights;
	std::vector<PBRMaterial> PBRMaterials;
	std::vector<PhongMaterial> PhongMaterials;
	std::map<std::string, SceneObject> objects;
	std::map<std::string, Texture> textures;

	int activeEnvironment = 0;
};

struct AppContext
{
	UserInput userInput;
	SceneContext scene;
	double globalTime = 0.0;
	double previousShaderCheckTime = 0.0;
	time_t previousModificationTime = 0;

	RenderContext *activeRC = nullptr;
	Shader activeShader;

	RenderContext sceneRC;
	RenderContext shadowRC;
	RenderContext texDisplayRC;

	std::map<Shader, GLuint> shaders;
	
	Model screenQuadModel;
	GLuint screenQuadProgram = 0;

	Model skyBoxModel;
};

namespace App
{
	void Init(AppContext *context, unsigned int screenWidth, unsigned int screenHeight);
	void Update(AppContext *context, double dt);
	void Release(AppContext *context);
}