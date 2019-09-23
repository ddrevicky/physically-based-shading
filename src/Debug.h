#pragma once

#include <glm/glm.hpp>

struct AppContext;
struct Texture;

namespace DEBUG
{
    void RenderVector(AppContext *context, glm::vec3 startPoint, glm::vec3 endPoint, glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.8f));
    void RenderWorldAxes(AppContext *context);
	void RenderCube(AppContext *context, glm::vec3 position, float edgeSize, glm::vec3 color);
    void RenderTexturedQuad(AppContext *context, Texture *texture); 
	void ReloadShadersIfModified(AppContext *context, const char *file);
}