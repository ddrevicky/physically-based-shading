#pragma once

#include "Def.h"
#include <glm/glm.hpp>

struct Mesh
{
	void *vertices = nullptr;
	unsigned int vertexCount = 0;
	size_t vertexStride = 0;
	
	void *indices = nullptr;
	unsigned int indexCount = 0;
	size_t indexStride = 0;
	
	std::vector<unsigned int> vertexAttributeSizes;
};

namespace UtilMesh
{
	Mesh MakeCubeCenteredWithNormals(float edgeSize);
	Mesh MakeScreenQuad();
	Mesh MakeFloor(float size = 15.0f);	
	Mesh MakeSkyBox();
	Mesh DEBUGVector(glm::vec3 startPoint, glm::vec3 endPoint, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));
	Mesh DEBUGWorldAxes(float axLength);	
	Mesh DEBUGMakeCube(float edgeSize, glm::vec3 color);
	Mesh MakeUVSphere(unsigned int subdivisions, float radius = 1.0f);
	Mesh MakeIcosahedronSphere(unsigned int recursionLevel = 2);
	void Free(Mesh mesh);
}