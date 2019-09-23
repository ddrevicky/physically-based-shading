#include <iostream>
#include <sys/types.h>
#include <cstdio>
#include <vector>
#include <math.h>

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"
#include "stb_image.h"

#include "App.h"
#include "IOUtil.h"
#include "Debug.h"
#include "Graphics.h"
#include "UtilMesh.h"
#include "Camera.h"
#include "UserInput.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using std::string;
using std::to_string;

static const int SPHERES_PER_ROW = 7;
static const int SPHERES_PER_COLUMN = 7;
static const char *RELOAD_SHADER = "../src/shaders/PBR.frag";

void BindRenderContext(AppContext *appContext, RenderContext *renderContext, Shader shader);
void CubemapFromTexture(AppContext *context, Shader shader, Texture *sampledTexture, Texture *cubemapTexture, unsigned int cubeMapSize);
void PrefilteredEnvMapFromTexture(AppContext *context, Shader shader, Texture *sampledTexture, Texture *prefilteredEnvMapTexture, unsigned int cubeMapSize);
void UpdateScene(AppContext *context, double dt);
void RenderScene(AppContext *context);
void BindRenderContext(AppContext *appContext, RenderContext *renderContext, Shader shader);
void RenderUI(AppContext *context);
void RenderDebugObjects(AppContext *context);
void RenderSkyBox(AppContext *appContext);

void InitSceneObjects(SceneContext *scene)
{
	scene->activeEnvironment = 0;
	//------------------------
	// Init Scene Objects
	//------------------------
	
	// Spheres
	int numSubdivisions = 64;
	float radius = 1.0f;
	float distanceBetweenSpheres = 2.0f * radius + 0.2f;

	for (int row = 0; row < SPHERES_PER_ROW; ++row)
	{
		for (int col = 0; col < SPHERES_PER_COLUMN; ++col)
		{
			int index = row * SPHERES_PER_COLUMN + col;

			std::string key = "sphere" + to_string(index);
			Mesh mesh = UtilMesh::MakeUVSphere(numSubdivisions, radius);
			unsigned int materialIndex = index;

			vec3 pos;
			pos.x = (-SPHERES_PER_ROW / 2 + col) * distanceBetweenSpheres;
			pos.y = radius + row * distanceBetweenSpheres;
			pos.z = 0.0f;
			mat4 modelMatrix = glm::translate(glm::mat4(), pos);

			scene->objects[key] = SceneObject(mesh, &scene->textures["albedo"], &scene->textures["metalness"], &scene->textures["roughness"],
											  &scene->textures["normal"], materialIndex, modelMatrix);

			// Create PBR material
			PBRMaterial PBRmaterial;
			PBRmaterial.albedo = vec3(0.5f, 0.0f, 0.0f);
			PBRmaterial.metalness = float(row) / float(SPHERES_PER_ROW); 
			PBRmaterial.roughness = glm::clamp(float(col) / float(SPHERES_PER_COLUMN), 0.05f, 1.0f);
			PBRmaterial.AO = 1.0f;
			scene->PBRMaterials.push_back(PBRmaterial);

			// Create Phong material
			PhongMaterial redPlastic;
			redPlastic.ambient = vec3(0.02f, 0.005f, 0.005f);
			redPlastic.diffuse = vec3(0.5f, 0.0f, 0.0f);
			redPlastic.specular = vec3(0.7f, 0.6f, 0.6f);
			redPlastic.shininess = 50.0f;
			scene->PhongMaterials.push_back(redPlastic);
		}
	}
	//scene->objects["icosphere"] = SceneObject(UtilMesh::MakeIcosahedronSphere(), Gold, 0);

	//------------------------
	// Init Lights
	//------------------------
	DirectionalLight *light = &scene->directionalLight;
	light->direction = glm::normalize(vec3(1.0f, -0.5f, 1.0f));
	light->ambient = vec3(0.2f, 0.2f, 0.2f);
	light->diffuse = vec3(0.5f, 0.5f, 0.5f);
	light->specular = vec3(1.0f, 1.0f, 1.0f);

	PointLight pointLight = {};
	pointLight.ambient = vec3(5.0f, 5.0f, 5.0f);
	pointLight.diffuse = vec3(25.0f, 25.0f, 25.0f);
	pointLight.specular = vec3(25.0f, 25.0f, 25.0f);

	float lightXOffset = (SPHERES_PER_ROW / 2) * distanceBetweenSpheres * 0.5f;
	float height = radius + SPHERES_PER_ROW * distanceBetweenSpheres;
	float distance = 4.0f;
	vec3 lightPositions[] =
	{
		vec3(-lightXOffset, 0.25f * height, distance),
		vec3(lightXOffset, 0.25f * height, distance),
		vec3(-lightXOffset, 0.75f  * height, distance),
		vec3(lightXOffset, 0.75f * height, distance),
	};

	for (unsigned int i = 0; i < ARRAYSIZE(lightPositions); ++i)
	{
		pointLight.position = lightPositions[i];
		scene->pointLights.push_back(pointLight);
	}
}

void App::Init(AppContext *context, unsigned int screenWidth, unsigned int screenHeight)
{	
	//------------------------
	// Init OpenGL State
	//------------------------
	Graphics::InitOpenGLState();

	//------------------------
	// Init Models
	//------------------------
	SceneContext *scene = &context->scene;								
	InitSceneObjects(scene);
	Graphics::InitModel(&context->screenQuadModel, UtilMesh::MakeScreenQuad());
	Graphics::InitModel(&context->skyBoxModel, UtilMesh::MakeSkyBox());

	//------------------------
	// Init Shaders
	//------------------------
	string shaderDir = "../src/shaders/";
	context->shaders[Shader::SkyBox] = Graphics::CreateProgram(shaderDir + "SkyBox.vert", shaderDir + "SkyBox.frag");
	context->shaders[Shader::EquirectToCubemap] = Graphics::CreateProgram(shaderDir + "CubeMap.vert", shaderDir + "EquirectToCubeMap.frag");
	context->shaders[Shader::EnvToIrradiance] = Graphics::CreateProgram(shaderDir + "CubeMap.vert", shaderDir + "EnvToIrradiance.frag");
	context->shaders[Shader::EnvToPrefilteredEnv] = Graphics::CreateProgram(shaderDir + "CubeMap.vert", shaderDir + "EnvToPrefilteredEnv.frag");
	context->shaders[Shader::EnvToIntegratedBRDF] = Graphics::CreateProgram(shaderDir + "TextureDisplay.vert", shaderDir + "EnvToIntegratedBRDF.frag");
	context->shaders[Shader::Debug] = Graphics::CreateProgram(shaderDir + "Debug.vert", shaderDir + "Debug.frag");;
	context->shaders[Shader::ShadowMap] = Graphics::CreateProgram(shaderDir + "Shadow.vert", shaderDir + "Shadow.frag");;

	{
		GLuint phongProgram = Graphics::CreateProgram(shaderDir + "Phong.vert", shaderDir + "Phong.frag");
		context->shaders[Shader::Phong] = phongProgram;

		Graphics::SetUniform3f(phongProgram, scene->directionalLight.direction, "uDirectionalLight.direction");
		Graphics::SetUniform3f(phongProgram, scene->directionalLight.ambient, "uDirectionalLight.ambient");
		Graphics::SetUniform3f(phongProgram, scene->directionalLight.diffuse, "uDirectionalLight.diffuse");
		Graphics::SetUniform3f(phongProgram, scene->directionalLight.specular, "uDirectionalLight.specular");

		for (unsigned int i = 0; i < scene->pointLights.size(); ++i)
		{
			
			Graphics::SetUniform3f(phongProgram, scene->pointLights[i].position, "uPointLights[" + to_string(i) + "].position");
			Graphics::SetUniform3f(phongProgram, scene->pointLights[i].ambient, "uPointLights[" + to_string(i) + "].ambient");
			Graphics::SetUniform3f(phongProgram, scene->pointLights[i].diffuse, "uPointLights[" + to_string(i) + "].diffuse");
			Graphics::SetUniform3f(phongProgram, scene->pointLights[i].specular, "uPointLights[" + to_string(i) + "].specular");
		}
	}

	{
		GLuint pbrProgram = Graphics::CreateProgram(shaderDir + "Phong.vert", shaderDir + "PBR.frag");
		context->shaders[Shader::PBR] = pbrProgram;

		for (unsigned int i = 0; i < scene->pointLights.size(); ++i)
		{
			Graphics::SetUniform3f(pbrProgram, scene->pointLights[i].position, "uPointLights[" + to_string(i) + "].position");
			Graphics::SetUniform3f(pbrProgram, scene->pointLights[i].ambient, "uPointLights[" + to_string(i) + "].ambient");
			Graphics::SetUniform3f(pbrProgram, scene->pointLights[i].diffuse, "uPointLights[" + to_string(i) + "].diffuse");
			Graphics::SetUniform3f(pbrProgram, scene->pointLights[i].specular, "uPointLights[" + to_string(i) + "].specular");
		}
#ifdef MATERIAL_TEXTURES
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::Albedo2D, "uTexAlbedo");
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::Metalness2D, "uTexMetalness");
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::Roughness2D, "uTexRoughness");
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::Normal2D, "uTexNormal");
#endif
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::IntegratedBRDF2D, "uTexIntegratedBRDF");
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::IrradianceMapCube, "uCubeIrradiance");
		Graphics::SetUniform1i(pbrProgram, PBRSamplers::PrefilteredEnvMapCube, "uCubePrefilteredEnvMap");
	}

	{
		GLuint textureDisplayProgram = Graphics::CreateProgram(shaderDir + "TextureDisplay.vert", shaderDir + "TextureDisplay.frag");
		context->shaders[Shader::TextureDisplay] = textureDisplayProgram;
		Graphics::SetUniform1i(textureDisplayProgram, 0, "uTexSampler0");
	}

	//------------------------
	// Init Render Contexts
	//------------------------

	// Scene 
	{
		Camera sceneCamera;
		vec3 sceneCamPosition = vec3(-17.48f, 11.28f, 10.24f);
		vec3 sceneCamTarget = vec3(0, 7.39f, 0);
		vec3 sceneCamUp = vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 sceneProjection = glm::perspective(45.0f, float(screenWidth) / float(screenHeight), 0.1f, 300.0f);
		CameraControl::SetView(&sceneCamera, sceneCamPosition, sceneCamTarget, sceneCamUp);	
		CameraControl::SetProjection(&sceneCamera, sceneProjection);
	
		RenderContext *sceneRC = &context->sceneRC;
		sceneRC->framebuffer.fbo = 0;
		sceneRC->framebuffer.width = screenWidth;
		sceneRC->framebuffer.height = screenHeight;
		sceneRC->viewport = Viewport(0, 0, screenWidth, screenHeight);
		sceneRC->camera = sceneCamera;
		IOUtil::GetFileModificationTime(RELOAD_SHADER, &context->previousModificationTime);
	}
	
	// Shadowmap
	{
		RenderContext *shadowRC = &context->shadowRC;
		unsigned int shadowMapSize = 2048;
		Graphics::InitDepthFramebuffer(&shadowRC->framebuffer, shadowMapSize, shadowMapSize);
		shadowRC->viewport = Viewport(0, 0, shadowMapSize, shadowMapSize);

		vec3 shadowCamPosition = 10.0f * -(scene->directionalLight.direction);
		vec3 shadowCamTarget = vec3(0.0f, 0.0f, 0.0f);
		vec3 shadowCamUp = vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 shadowProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
		CameraControl::SetView(&shadowRC->camera, shadowCamPosition, shadowCamTarget, shadowCamUp);
		CameraControl::SetProjection(&shadowRC->camera, shadowProjection);
	}

	// TODO: Do not need viewmatrix, projectionmatrix but BindRendercontext will still try to set these uniforms
	// If this was solved on init it would not be a problem.
	// Texture Display
	{
		RenderContext *texDisplayRC = &context->texDisplayRC;
		texDisplayRC->camera.viewMatrix = glm::mat4();
		texDisplayRC->camera.projectionMatrix = glm::mat4();
		unsigned int viewportWidth = 400;
		unsigned int viewportHeight = 300;
		texDisplayRC->viewport = Viewport(screenWidth - viewportWidth - 1, screenHeight - viewportHeight - 1,
										  viewportWidth, viewportHeight);
	}

	//------------------------
	// Init Textures 
	//------------------------
	
	// Object textures
#ifdef MATERIAL_TEXTURES
	{
		Texture tex;
		Graphics::InitTexture2D(&tex, "../resources/rusted_iron/albedo.png");
		scene->textures["albedo"] = tex;
		Graphics::InitTexture2D(&tex, "../resources/rusted_iron/metallic.png");
		scene->textures["metalness"] = tex;
		Graphics::InitTexture2D(&tex, "../resources/rusted_iron/roughness.png");
		scene->textures["roughness"] = tex;
		Graphics::InitTexture2D(&tex, "../resources/rusted_iron/normal.png");
		scene->textures["normal"] = tex;
	}
#endif

	// HDR Environment Textures
	{
		const char *hdrTexturePaths[] =
		{
			"../resources/hdr/newport_loft.hdr",
			"../resources/hdr/Ditch-River_2k.hdr"
		};

		for (unsigned int i = 0; i < ARRAYSIZE(hdrTexturePaths); ++i)
		{
			Texture hdrTexture;
			Texture *environmentTexture = &scene->textures["skybox" + to_string(i)];
			Graphics::InitHDRTexture(&hdrTexture, hdrTexturePaths[i]);
			// Convert 2D HDR equirectangular environment map to environment cubemap
			CubemapFromTexture(context, Shader::EquirectToCubemap, &hdrTexture, environmentTexture, 512);
			Graphics::Release(&hdrTexture);
			// Convert environment cubemap to irradiance cubemap
			CubemapFromTexture(context, Shader::EnvToIrradiance, environmentTexture, &scene->textures["irradianceMap" + to_string(i)], 32);
			// Convert environment cubemap to prefiltered environment cubemap
			PrefilteredEnvMapFromTexture(context, Shader::EnvToPrefilteredEnv, environmentTexture, &scene->textures["prefilteredEnvMap" + to_string(i)], 128);
		}
	}

	// Integrated BRDF 2D LUT
	{
		RenderContext screenQuadRC = {};
		unsigned int BRDFMapSize = 512;
		Graphics::InitDrawFramebuffer(&screenQuadRC.framebuffer, BRDFMapSize, BRDFMapSize, GL_RG16F, GL_RG);
		screenQuadRC.viewport = Viewport(0, 0, BRDFMapSize, BRDFMapSize);
		Graphics::BindRenderContext(&screenQuadRC, context->shaders[Shader::EnvToIntegratedBRDF]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Graphics::RenderModel(&context->screenQuadModel, context->shaders[Shader::EnvToIntegratedBRDF]);

		scene->textures["integratedBRDF"].id = screenQuadRC.framebuffer.colorAttachment.id;
		scene->textures["integratedBRDF"].target = TextureTarget::Texture2D;
		screenQuadRC.framebuffer.colorAttachment.id = 0;
		Graphics::Release(&screenQuadRC);
	}
}

void App::Update(AppContext *context, double dt)
{
	context->globalTime += dt;
	UpdateScene(context, dt);

	// Clear render contexts
	Graphics::ClearRenderContext(&context->sceneRC, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Graphics::ClearRenderContext(&context->shadowRC, GL_DEPTH_BUFFER_BIT);

	// Render shadowmap
	BindRenderContext(context, &context->shadowRC, ShadowMap);
	RenderScene(context);

	// Render scene
#if 0	// Phong
	BindRenderContext(context, &context->sceneRC, Shader::Phong);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, context->shadowRC.framebuffer.depthAttachment.id);
	GLuint phongProgram = context->shaders[Shader::Phong];
	Graphics::SetUniform1i(phongProgram, PhongSamplers::Shadowmap2D, "uTexSampler0");
	glm::mat4 lightViewProjectionMatrix = context->shadowRC.camera.projectionMatrix * context->shadowRC.camera.viewMatrix;
	Graphics::SetMatrixUniform(phongProgram, lightViewProjectionMatrix, "uLightViewProjectionMatrix");
	Graphics::SetUniform3f(phongProgram, context->sceneRC.camera.position, "uCameraPosWorld");
	RenderScene(context);
#else	// PBR
	BindRenderContext(context, &context->sceneRC, Shader::PBR);
	Graphics::SetUniform3f(context->shaders[Shader::PBR], context->sceneRC.camera.position, "uCameraPosWorld");
	RenderScene(context);
#endif
	RenderSkyBox(context);

#ifdef _DEBUG
	RenderDebugObjects(context);
#endif
	RenderUI(context);
}

void CubemapFromTexture(AppContext *context, Shader shader, Texture *sampledTexture, Texture *cubemapTexture, unsigned int cubeMapSize)
{
	RenderContext cubeMapRC;
	Graphics::InitCubeMapFramebuffer(&cubeMapRC.framebuffer, cubeMapSize, cubeMapSize, false);
	cubeMapRC.viewport = Viewport(0, 0, cubeMapSize, cubeMapSize);
	
	// cubeMapFaces order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
	const unsigned int numberOfCubeFaces = 6;
	mat4 cubeMapViewMatrices[numberOfCubeFaces] =
	{
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f)),
	};

	BindRenderContext(context, &cubeMapRC, shader);
	for (unsigned int i = 0; i < numberOfCubeFaces; ++i)
	{
		cubeMapRC.camera.projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		cubeMapRC.camera.viewMatrix = cubeMapViewMatrices[i];
		CameraControl::Use(&cubeMapRC.camera, context->shaders[shader]);

		Graphics::BindTexture(sampledTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapRC.framebuffer.colorAttachment.id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Graphics::RenderModel(&context->skyBoxModel, context->shaders[shader]);
	}

	cubemapTexture->id = cubeMapRC.framebuffer.colorAttachment.id;
	cubemapTexture->target = TextureTarget::Cubemap;
	cubeMapRC.framebuffer.colorAttachment.id = 0;
	Graphics::Release(&cubeMapRC);
}

void PrefilteredEnvMapFromTexture(AppContext *context, Shader shader, Texture *sampledTexture, Texture *prefilteredEnvMapTexture, unsigned int cubeMapSize)
{
	RenderContext cubeMapRC;
	Graphics::InitCubeMapFramebuffer(&cubeMapRC.framebuffer, cubeMapSize, cubeMapSize, true);
	cubeMapRC.viewport = Viewport(0, 0, cubeMapSize, cubeMapSize);
	BindRenderContext(context, &cubeMapRC, shader);
	Graphics::BindTexture(sampledTexture, 0);

	// cubeMapFaces order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
	const unsigned int numberOfCubeFaces = 6;
	mat4 cubeMapViewMatrices[numberOfCubeFaces] =
	{
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f)),
	};

	unsigned int maxMipLevels = 5;
	for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel)
	{
		unsigned int mipSize = (unsigned int)(cubeMapSize * pow(0.5f, mipLevel));
		cubeMapRC.viewport = Viewport(0, 0, mipSize, mipSize);
		float roughness = float(mipLevel) / float(maxMipLevels - 1);
		Graphics::SetUniform1f(context->shaders[shader], roughness, "uRoughness");
		glBindRenderbuffer(GL_RENDERBUFFER, cubeMapRC.framebuffer.depthAttachment.id);
		glRenderbufferStorage(GL_RENDERBUFFER, cubeMapRC.framebuffer.depthAttachment.internalFormat, mipSize, mipSize);

		for (unsigned int face = 0; face < numberOfCubeFaces; ++face)
		{
			//cubeMapRC.camera.projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			cubeMapRC.camera.viewMatrix = cubeMapViewMatrices[face];
			Graphics::BindRenderContext(&cubeMapRC, context->shaders[shader]);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubeMapRC.framebuffer.colorAttachment.id, mipLevel);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Graphics::RenderModel(&context->skyBoxModel, context->shaders[shader]);
		}
	}

	prefilteredEnvMapTexture->id = cubeMapRC.framebuffer.colorAttachment.id;
	prefilteredEnvMapTexture->target = TextureTarget::Cubemap;
	cubeMapRC.framebuffer.colorAttachment.id = 0;
	Graphics::Release(&cubeMapRC);
}

void UpdateScene(AppContext *context, double dt)
{
	CameraControl::UpdateCamera(&context->sceneRC.camera, dt, &context->userInput);
	
	// This was necessary because of SDL behavior on Linux (pressing a key would generate double keypressed message)
	double static lastEnvironmentChangeTime = 0.0;
	if (context->userInput.qPressed && context->globalTime - lastEnvironmentChangeTime > 0.05)	
	{
		lastEnvironmentChangeTime = context->globalTime;
		context->scene.activeEnvironment = context->scene.activeEnvironment == 0 ? 1 : 0;
	}

	UserInput cleanUserInput = {};
	context->userInput = cleanUserInput;
}

void RenderScene(AppContext *context)
{
	GLuint program = context->shaders[context->activeShader];
	int i = 0;
	for (auto it : context->scene.objects)
	{
		if (context->activeShader == Shader::Phong)
		{
			PhongMaterial PhongMat = context->scene.PhongMaterials[it.second.materialIndex];
			Graphics::SetUniform3f(program, PhongMat.ambient, "uMaterial.ambient");
			Graphics::SetUniform3f(program, PhongMat.diffuse, "uMaterial.diffuse");
			Graphics::SetUniform3f(program, PhongMat.specular, "uMaterial.specular");
			Graphics::SetUniform1f(program, PhongMat.shininess, "uMaterial.shininess");
		}
		else if (context->activeShader == Shader::PBR)
		{
			PBRMaterial PBRmat = context->scene.PBRMaterials[it.second.materialIndex];
			Graphics::SetUniform3f(program, PBRmat.albedo, "uAlbedo");
			Graphics::SetUniform1f(program, PBRmat.metalness, "uMetalness");
			Graphics::SetUniform1f(program, PBRmat.roughness, "uRoughness");
			Graphics::SetUniform1f(program, PBRmat.AO, "uAO");
#ifdef MATERIAL_TEXTURES
			Graphics::BindTexture(it.second.albedoTexture, PBRSamplers::Albedo2D);
			Graphics::BindTexture(it.second.metalnessTexture, PBRSamplers::Metalness2D);
			Graphics::BindTexture(it.second.roughnessTexture, PBRSamplers::Roughness2D);
			Graphics::BindTexture(it.second.normalTexture, PBRSamplers::Normal2D);
#endif
			Graphics::BindTexture(&context->scene.textures["integratedBRDF"], PBRSamplers::IntegratedBRDF2D);
			Graphics::BindTexture(&context->scene.textures["irradianceMap" + to_string(context->scene.activeEnvironment)], PBRSamplers::IrradianceMapCube);
			Graphics::BindTexture(&context->scene.textures["prefilteredEnvMap" + to_string(context->scene.activeEnvironment)], PBRSamplers::PrefilteredEnvMapCube);
		}

		Graphics::RenderModel(&it.second.model, program, it.second.modelMatrix);	
		++i;
	}
}

void RenderSkyBox(AppContext *context)
{
	BindRenderContext(context, &context->sceneRC, Shader::SkyBox);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	//Graphics::BindTexture(&context->scene.textures["irradianceMap" + to_string(context->scene.activeEnvironment)], 0);
	//Graphics::BindTexture(&context->scene.textures["prefilteredEnvMap" + to_string(context->scene.activeEnvironment)], 0);
	Graphics::BindTexture(&context->scene.textures["skybox" + to_string(context->scene.activeEnvironment)], 0);
	Graphics::RenderModel(&context->skyBoxModel, context->shaders[Shader::SkyBox]);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

void BindRenderContext(AppContext *appContext, RenderContext *renderContext, Shader shader)
{
	appContext->activeRC = renderContext;
	appContext->activeShader = shader;
	GLuint activeProgram = appContext->shaders[shader];
	Graphics::BindRenderContext(renderContext, activeProgram);
}

void RenderUI(AppContext *context)
{
	SceneContext *scene = &context->scene;
	
	ImGui::SetNextWindowSize(ImVec2(10, 10), ImGuiSetCond_Appearing);
	ImGui::Begin("PBR", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::SetWindowSize(ImVec2(170, 70), ImGuiSetCond_Always);

	ImGui::Text("W/S - Shift camera");
	ImGui::Text("Q - Cycle environment");

	ImGui::End();
#if 0
	bool sliderUsed = false;
	sliderUsed = ImGui::SliderFloat("Red Plastic", &scene->phongMaterials[RedPlastic].shininess, 0, 400.0f);
	sliderUsed = ImGui::SliderFloat("Emerald", &scene->phongMaterials[Emerald].shininess, 0, 400.0f) || sliderUsed;
	if (sliderUsed)
	{
		GLuint phongProgram = context->shaders[Shader::Phong];
		Graphics::SetUniform1f(phongProgram, scene->phongMaterials[RedPlastic].shininess, "uMaterials[" + to_string(RedPlastic) + "].shininess");
		Graphics::SetUniform1f(phongProgram, scene->phongMaterials[Emerald].shininess, "uMaterials[" + to_string(Emerald) + "].shininess");
		Graphics::SetUniform1f(phongProgram, scene->phongMaterials[Gold].shininess, "uMaterials[" + to_string(Gold) + "].shininess");
	}
#endif
}

void RenderDebugObjects(AppContext *context)
{
	DEBUG::ReloadShadersIfModified(context, RELOAD_SHADER);
	DEBUG::RenderWorldAxes(context);
	for (unsigned int i = 0; i < context->scene.pointLights.size(); ++i)
	{
		DEBUG::RenderCube(context, context->scene.pointLights[i].position, 0.3f, vec3(1.0f, 1.0f, 1.0f));
	}
	DEBUG::RenderTexturedQuad(context, &context->scene.textures["albedo"]);
}

void App::Release(AppContext *context)
{
	Graphics::Release(&context->sceneRC);
	Graphics::Release(&context->shadowRC);
	Graphics::Release(&context->texDisplayRC);

	Graphics::Release(&context->screenQuadModel);
	Graphics::Release(&context->skyBoxModel);

	for (auto it : context->scene.textures)
	{
		Graphics::Release(&it.second);
	}

	for (auto it : context->scene.objects)
	{
		Graphics::Release(&it.second.model);
	}

	for (auto it : context->shaders)
	{
		Graphics::Release(it.second);
	}
}