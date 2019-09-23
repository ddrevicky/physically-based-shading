#pragma once

#include <vector>
#include <string>

#include <glad/glad.h> 
#include <glm/glm.hpp>

#include "Camera.h"
#include "Def.h"

#define glCheckError() glCheckError_(__FILE__, __LINE__) 

#define VIEW_UNIFORM_NAME "uViewMatrix"
#define MODEL_UNIFORM_NAME "uModelMatrix"
#define PROJECTION_UNIFORM_NAME "uProjectionMatrix"

struct Mesh;

struct Model
{
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ibo = 0;

	GLuint indexCount = 0;
	GLuint indexStride = 0;
};

enum FramebufferAttachmentType
{
	TextureAttachment,
	RenderbufferAttachment
};

struct FramebufferAttachment
{
	GLuint id = 0;
	FramebufferAttachmentType type;
	GLenum internalFormat = 0;
	GLenum format = 0;
};

struct Framebuffer
{
	GLuint fbo = 0;
	FramebufferAttachment colorAttachment;
	FramebufferAttachment depthAttachment;
	unsigned int width = 0;
	unsigned int height = 0;
};

enum TextureTarget
{
	Texture2D,
	Cubemap
};

struct Texture
{
	GLuint id = 0;
	TextureTarget target = Texture2D;
};

struct Viewport
{
	unsigned int bottomX = 0;
	unsigned int bottomY = 0;
	unsigned int width = 0;
	unsigned int height = 0;

	Viewport()
	{
	}

	Viewport(unsigned int bottomX, unsigned int bottomY, unsigned int width, unsigned int height)
	{
		this->bottomX = bottomX;
		this->bottomY = bottomY;
		this->width = width;
		this->height = height;
	}
};

struct RenderContext
{
	Camera camera;
	Viewport viewport;
	Framebuffer framebuffer;
};

namespace Graphics
{
	void InitOpenGLState();
	void InitModel(Model *model, Mesh mesh);
	void InitTexture2D(Texture *texture, uint8_t *data, unsigned int width, unsigned int height, GLenum internalFormat, GLenum format);
	void InitTexture2D(Texture *texture, uint8_t *data, unsigned int width, unsigned int height, unsigned int numComponents);
	void InitTexture2D(Texture *texture, const char *sourceFile);
	void InitHDRTexture(Texture *texture, const char *sourceFile);
	void InitCubemapTexture(Texture *texture, std::vector<unsigned char *>, std::vector<unsigned int> widths, std::vector<unsigned int> heights, unsigned int numChannels);
	void InitCubemapTexture(Texture *texture, std::vector<std::string> cubeMapFaces);
	void InitDrawFramebuffer(Framebuffer *framebuffer, unsigned int width, unsigned int height, GLenum colorInternalFormat = GL_RGB, GLenum colorFormat = GL_RGB);
	void InitCubeMapFramebuffer(Framebuffer *framebuffer, unsigned int width, unsigned int height, bool useMipMaps);
	void InitDepthFramebuffer(Framebuffer *framebuffer, unsigned int width, unsigned int height);
	bool CreateShader(GLenum shaderType, GLuint *shader, std::string shaderSourceFile);
	GLuint CreateProgram(std::string vertexShaderFile, std::string fragmentShaderFile);

	void BindTexture(Texture *texture, unsigned int slot);
	void UseProgram(GLuint program);
	void RenderModel(Model *model, GLuint program, glm::mat4 modelMatrix = glm::mat4());
	void SetMatrixUniform(GLuint program, glm::mat4 matrix, std::string uniformName);
	void SetUniform1i(GLuint program, int value, std::string uniformName);
	void SetUniform1f(GLuint program, float value, std::string uniformName);
	void SetUniform2f(GLuint program, float v1, float v2, std::string uniformName);
	void SetUniform3f(GLuint program, glm::vec3 v, std::string uniformName);

	void ClearRenderContext(RenderContext *renderContext, GLbitfield mask);
	void BindRenderContext(RenderContext *renderContext, GLuint program);

	void Release(RenderContext *renderContext);
	void Release(Model *model);
	void Release(GLuint program);
	void Release(Framebuffer *framebuffer);
	void Release(Texture *texture);
}