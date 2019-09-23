#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Debug.h"
#include "App.h"
#include "Graphics.h"
#include "UtilMesh.h"
#include "IOUtil.h"

using glm::vec3;
using glm::mat4;

static void Render(AppContext *context, Mesh mesh, mat4 modelMatrix = glm::mat4())
{
	Model model;

	GLuint debugProgram = context->shaders[Shader::Debug];

	Graphics::InitModel(&model, mesh);
	Graphics::BindRenderContext(&context->sceneRC, debugProgram);
	Graphics::UseProgram(debugProgram);
	CameraControl::Use(&context->sceneRC.camera, debugProgram);
	Graphics::RenderModel(&model, debugProgram, modelMatrix);
	Graphics::UseProgram(0);
	Graphics::Release(&model);
}

void DEBUG::RenderVector(AppContext *context, glm::vec3 startPoint, glm::vec3 endPoint, glm::vec3 color)
{
	Render(context, UtilMesh::DEBUGVector(startPoint, endPoint, color));
}

void DEBUG::RenderWorldAxes(AppContext *context)
{
	float length = 2.0f;
	Render(context, UtilMesh::DEBUGWorldAxes(length));
}

void DEBUG::RenderCube(AppContext *context, glm::vec3 position, float edgeSize, glm::vec3 color)
{
	mat4 modelMatrix = glm::translate(glm::mat4(), position);
	Render(context, UtilMesh::DEBUGMakeCube(edgeSize, color), modelMatrix);
}

void DEBUG::RenderTexturedQuad(AppContext *context, Texture *texture)
{
	GLuint textureDisplayProgram = context->shaders[Shader::TextureDisplay];
	Graphics::BindRenderContext(&context->texDisplayRC, textureDisplayProgram);	
	glDepthFunc(GL_ALWAYS);

	Graphics::BindTexture(texture, 0);
	Graphics::RenderModel(&context->screenQuadModel, textureDisplayProgram);

	glDepthFunc(GL_LESS);
}

// Shader uniforms have to be reset, app state is completely lost
void DEBUG::ReloadShadersIfModified(AppContext *context, const char *file)
{
	double checkEvery = 0.5;
	if (context->globalTime - context->previousShaderCheckTime > checkEvery)
	{
		time_t modificationTime;
		if (!IOUtil::GetFileModificationTime(file, &modificationTime))
		{
			std::cerr << "Error getting file write time for " << file << "\n";
			return;
		}

		if (modificationTime != context->previousModificationTime)
		{
			unsigned int screenWidth = context->sceneRC.framebuffer.width;
			unsigned int screenHeight = context->sceneRC.framebuffer.height;
			App::Release(context);
			App::Init(context, screenWidth, screenHeight);

			context->previousModificationTime = modificationTime;
		}
		context->previousShaderCheckTime = context->globalTime;
	}
}