#include <iostream>
#include <ios>
#include <string>
#include <fstream>
#include <vector>

#include <glad/glad.h> 
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include "Graphics.h"
#include "UtilMesh.h"

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			//case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;		// GLAD didn't know these (GLEW did)
			//case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;		
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

void Graphics::InitModel(Model *model, Mesh mesh)
{
	glGenVertexArrays(1, &model->vao);
	glBindVertexArray(model->vao);

	glGenBuffers(1, &model->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertexCount*mesh.vertexStride, mesh.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &model->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexCount*mesh.indexStride, mesh.indices, GL_STATIC_DRAW);
	
	size_t offset = 0;
	for (unsigned int i = 0; i < mesh.vertexAttributeSizes.size(); ++i)
	{
		glEnableVertexAttribArray(i);
		unsigned int attributeSize = mesh.vertexAttributeSizes[i];
		glVertexAttribPointer(i, attributeSize, GL_FLOAT, GL_FALSE, mesh.vertexStride, (void*)offset);
		offset += sizeof(float) * attributeSize;
	}

	model->indexCount = mesh.indexCount;
	model->indexStride = mesh.indexStride;

	UtilMesh::Free(mesh);
	glBindVertexArray(0);
	glCheckError(); 	
}

bool Graphics::CreateShader(GLenum shaderType, GLuint *shader, std::string shaderSourceFile)
{
	*shader = glCreateShader(shaderType);
	if (*shader == 0)
	{
		std::cerr << "glCreateShader failed. \n";
		return false;
	}

	std::ifstream f(shaderSourceFile);
	if (!f.good())
	{
		std::cerr << "File" << shaderSourceFile << "does not exist\n";
		return false;
	}

	
	std::ifstream shaderFile;
	shaderFile.open(shaderSourceFile, std::ios::binary | std::ios::in | std::ios::ate);

	if (!shaderFile.is_open())
	{
		std::cerr << "Unable to open file\n";
		return false;
	}

	std::streampos size = shaderFile.tellg();
	int intSize = int(size);
	char *data = new char[intSize];
	shaderFile.seekg(0, std::ios::beg);
	shaderFile.read(data, size);
	shaderFile.close();

	glShaderSource(*shader, 1, (GLchar **)&data, (GLint *)&intSize);
	glCompileShader(*shader);
	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logBuffer[512];
		glGetShaderInfoLog(*shader, 512, NULL, logBuffer);
		std::cerr << "glCompileShader failed.\n";
		std::cerr << logBuffer;
	}
	delete[] data;

	glCheckError();
	return true;
}

GLuint Graphics::CreateProgram(std::string vertexShaderFile, std::string fragmentShaderFile)
{
	GLuint vertexShader, fragmentShader;
	CreateShader(GL_VERTEX_SHADER, &vertexShader, vertexShaderFile);
	CreateShader(GL_FRAGMENT_SHADER, &fragmentShader, fragmentShaderFile);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glCheckError();

	GLint status;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		std::cerr << "ERROR: Program linking failed. VS: " << vertexShaderFile << " FS: " << fragmentShaderFile << "\n";
	}

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);

	return shaderProgram;
}

void Graphics::InitTexture2D(Texture *texture, uint8_t *data, unsigned int width, unsigned int height, GLenum internalFormat, GLenum format)
{
	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Graphics::InitTexture2D(Texture *texture, uint8_t *data, unsigned int width, unsigned int height, unsigned int numChannels)
{
	GLenum internalFormat = GL_RGBA, format = GL_RGBA;

	if (numChannels == 1)
	{
		format = GL_RED;
		internalFormat = GL_RED;
	}
	else if (numChannels == 2)
	{
		format = GL_RG;
		internalFormat = GL_RG;
	}
	else if (numChannels == 3)
	{
		format = GL_RGB;
		internalFormat = GL_RGB;
	}
	else if (numChannels == 4)
	{
		format = GL_RGBA;
		internalFormat = GL_RGBA;
	}
	else
	{
		std::cerr << "ERROR: Unknown number of channels. " << numChannels << "\n";
	}

	Graphics::InitTexture2D(texture, data, width, height, internalFormat, format);
}

void Graphics::InitTexture2D(Texture *texture, const char *sourceFile)
{
	int width, height, numChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(sourceFile, &width, &height, &numChannels, 0);
	if (data)
	{
		Graphics::InitTexture2D(texture, data, width, height, numChannels);
	}
	else
	{
		std::cerr << "ERROR: Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void Graphics::InitHDRTexture(Texture *texture, const char *sourceFile)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, numComponents;
	float *data = stbi_loadf(sourceFile, &width, &height, &numComponents, 0);
	if (data)
	{
		glGenTextures(1, &texture->id);
		glBindTexture(GL_TEXTURE_2D, texture->id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Failed to load HDR image." << std::endl;
	}
}

// cubeMapFaces order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
void Graphics::InitCubemapTexture(Texture *texture, std::vector<unsigned char *> texData, std::vector<unsigned int> widths, 
								  std::vector<unsigned int> heights, unsigned int numChannels)
{
	const unsigned int numberOfCubeMapFaces = 6;
	if (texData.size() != numberOfCubeMapFaces)
	{
		std::cerr << "ERROR: Unknown number of channels. " << numChannels << "\n";
	}

	GLenum format = GL_RGBA;
	if (numChannels == 1)
		format = GL_RED;
	else if (numChannels == 3)
		format = GL_RGB;
	else if (numChannels == 4)
		format = GL_RGBA;
	else
	{
		std::cerr << "ERROR: Unknown number of channels. " << numChannels << "\n";
		return;
	}

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture->id);
		for (int i = 0; i < numberOfCubeMapFaces; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, widths[i], heights[i], 0, format, GL_UNSIGNED_BYTE, texData[i]);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// cubeMapFaces order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
void Graphics::InitCubemapTexture(Texture *texture, std::vector<std::string> cubeMapFaces)
{
	stbi_set_flip_vertically_on_load(false);

	std::vector<unsigned char *> texData;
	std::vector<unsigned int> widths;
	std::vector<unsigned int> heights;
	int numChannels;
	for (unsigned int i = 0; i < cubeMapFaces.size(); ++i)
	{
		int width, height;
		unsigned char *data = stbi_load(cubeMapFaces[i].c_str(), &width, &height, &numChannels, 0);
		if (data)
		{
			texData.push_back(data);
			widths.push_back(width);
			heights.push_back(height);
		}
		else
		{
			std::cerr << "Cubemap texture failed to load at path: " << cubeMapFaces[i] << std::endl;
			for (unsigned int i = 0; i < texData.size(); ++i)
				stbi_image_free(texData[i]);
			return;
		}
	}
	Graphics::InitCubemapTexture(texture, texData, widths, heights, numChannels);

	for (unsigned int i = 0; i < texData.size(); ++i)
		stbi_image_free(texData[i]);
}

void Graphics::BindTexture(Texture *texture, unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	GLenum target;
	switch (texture->target)
	{
		case TextureTarget::Texture2D:
			target = GL_TEXTURE_2D;
			break;
		case TextureTarget::Cubemap:
			target = GL_TEXTURE_CUBE_MAP;
			break;
		default:
			return;
	}
	glBindTexture(target, texture->id);
	glCheckError();
}

void Graphics::InitOpenGLState()
{
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
}

void Graphics::UseProgram(GLuint program)
{
	glUseProgram(program);
}

void Graphics::RenderModel(Model *model, GLuint program, glm::mat4 modelMatrix)
{
	Graphics::SetMatrixUniform(program, modelMatrix, MODEL_UNIFORM_NAME);
	GLenum indexType;
	switch (model->indexStride)
	{
		case 2:
			indexType = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indexType = GL_UNSIGNED_INT;
			break;
		default:
			std::cerr << "Index stride not recognized.\n";
			return;
			break;
	}

	glBindVertexArray(model->vao);
		glDrawElements(GL_TRIANGLES, model->indexCount, indexType, 0);
	glBindVertexArray(0);
	glCheckError();
}

void Graphics::InitDepthFramebuffer(Framebuffer *framebuffer, unsigned int width, unsigned int height)
{
	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->depthAttachment.type = FramebufferAttachmentType::TextureAttachment;
	framebuffer->depthAttachment.internalFormat = GL_DEPTH_COMPONENT;
	framebuffer->depthAttachment.format = GL_DEPTH_COMPONENT;

	glGenFramebuffers(1, &framebuffer->fbo);
	glGenTextures(1, &framebuffer->depthAttachment.id);
	glBindTexture(GL_TEXTURE_2D, framebuffer->depthAttachment.id);
	glTexImage2D(GL_TEXTURE_2D, 0, framebuffer->depthAttachment.internalFormat,
				 width, height, 0, framebuffer->depthAttachment.format, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer->depthAttachment.id, 0);
		glDrawBuffer(GL_NONE);			// Do not render to the color buffer
		glReadBuffer(GL_NONE);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	
	glCheckError();
}

void Graphics::InitDrawFramebuffer(Framebuffer *framebuffer, unsigned int width, unsigned int height, GLenum colorInternalFormat, GLenum colorFormat)
{
	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->depthAttachment.type = FramebufferAttachmentType::RenderbufferAttachment;
	framebuffer->depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
	framebuffer->depthAttachment.format = 0;
	framebuffer->colorAttachment.type = FramebufferAttachmentType::TextureAttachment;
	framebuffer->colorAttachment.internalFormat = colorInternalFormat;
	framebuffer->colorAttachment.format = colorFormat;

	glGenFramebuffers(1, &framebuffer->fbo);
	glGenRenderbuffers(1, &framebuffer->depthAttachment.id);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->fbo);
		// Depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->depthAttachment.id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer->depthAttachment.id);

		// Color buffer
		glGenTextures(1, &framebuffer->colorAttachment.id);
		glBindTexture(GL_TEXTURE_2D, framebuffer->colorAttachment.id);
		glTexImage2D(GL_TEXTURE_2D, 0, colorInternalFormat, width, height, 0, colorFormat, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->colorAttachment.id, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glCheckError();
}

void Graphics::InitCubeMapFramebuffer(Framebuffer *framebuffer, unsigned int width, unsigned int height, bool useMipMaps)
{
	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->depthAttachment.type = FramebufferAttachmentType::RenderbufferAttachment;
	framebuffer->depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
	framebuffer->depthAttachment.format = 0;
	framebuffer->colorAttachment.type = FramebufferAttachmentType::TextureAttachment;
	framebuffer->colorAttachment.internalFormat = GL_RGB16F;
	framebuffer->colorAttachment.format = GL_RGB;

	GLint textureMinFilter = GL_LINEAR;
	if (useMipMaps)
		textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;

	glGenFramebuffers(1, &framebuffer->fbo);
	glGenRenderbuffers(1, &framebuffer->depthAttachment.id);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->depthAttachment.id);
		glRenderbufferStorage(GL_RENDERBUFFER, framebuffer->depthAttachment.internalFormat, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer->depthAttachment.id);

		glGenTextures(1, &framebuffer->colorAttachment.id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, framebuffer->colorAttachment.id);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, framebuffer->colorAttachment.internalFormat, width, height, 
						 0, framebuffer->colorAttachment.format, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, textureMinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (useMipMaps)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::SetUniform1i(GLuint program, int value, std::string uniformName)
{
	glUseProgram(program);
	GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
	if (uniformLocation == -1)
	{
		return;
	}
	glUniform1i(uniformLocation, value);
	glCheckError();
}

void Graphics::SetUniform1f(GLuint program, float value, std::string uniformName)
{
	glUseProgram(program);
	GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
	if (uniformLocation == -1)
	{
		return;
	}
	glUniform1f(uniformLocation, value);
	glCheckError();
}

void Graphics::SetUniform2f(GLuint program, float v1, float v2, std::string uniformName)
{
	glUseProgram(program);
	GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
	if (uniformLocation == -1)
	{
		return;
	}
	glUniform2f(uniformLocation, v1, v2);
	glCheckError();
}

void Graphics::SetUniform3f(GLuint program, glm::vec3 v, std::string uniformName)
{
	glUseProgram(program);
	GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
	if (uniformLocation == -1)
	{
		return;
	}
	glUniform3f(uniformLocation, v.x, v.y, v.z);
	glCheckError();
}

void Graphics::SetMatrixUniform(GLuint program, glm::mat4 matrix, std::string uniformName)
{
	glUseProgram(program);
	GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
	if (uniformLocation == -1)
	{
		return;
	}
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(matrix));
	glCheckError();
}

void Graphics::ClearRenderContext(RenderContext *renderContext, GLbitfield mask)
{
	glBindFramebuffer(GL_FRAMEBUFFER, renderContext->framebuffer.fbo);	
	glClear(mask);
}

void Graphics::BindRenderContext(RenderContext *renderContext, GLuint program)
{
	glBindFramebuffer(GL_FRAMEBUFFER, renderContext->framebuffer.fbo);
	glViewport(renderContext->viewport.bottomX, renderContext->viewport.bottomY, 
			   renderContext->viewport.width, renderContext->viewport.height);
	Graphics::UseProgram(program);
	CameraControl::Use(&renderContext->camera, program);
}

void Graphics::Release(RenderContext *renderContext)
{
	if (!renderContext->framebuffer.fbo != 0)
	{
		Graphics::Release(&renderContext->framebuffer);
	}
}

void Graphics::Release(Model *model)
{
	glDeleteBuffers(1, &model->vbo);
	glDeleteBuffers(1, &model->ibo);
	glDeleteVertexArrays(1, &model->vao);
}

void Graphics::Release(GLuint program)
{
	glDeleteProgram(program);
}

void Graphics::Release(Framebuffer *framebuffer)
{
	glDeleteFramebuffers(1, &framebuffer->fbo);
	if (framebuffer->depthAttachment.id != 0)
		glDeleteTextures(1, &framebuffer->depthAttachment.id);
	if (framebuffer->colorAttachment.id != 0)
		glDeleteTextures(1, &framebuffer->colorAttachment.id);
}

void Graphics::Release(Texture *texture)
{
	glDeleteTextures(1, &texture->id);
}