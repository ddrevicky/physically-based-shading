#include <vector>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <limits> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "UtilMesh.h"

using std::vector;
using glm::vec2;
using glm::vec3;
using glm::mat4;
using glm::dot;
using glm::abs;
using glm::cross;
using glm::normalize;

#define PI 3.14159265358979323846f

#define POSITION_SIZE 3
#define NORMAL_SIZE 3
#define TEXCOORD_SIZE 2
#define COLOR_SIZE 3

Mesh ConcatenateMeshes(std::vector<Mesh> meshes);

template<typename TVertex>
void AddVertex(Mesh *mesh, TVertex v)
{
	((TVertex*)(mesh->vertices))[mesh->vertexCount++] = v;
}

template<typename TVertex>
void AddVertexTriangle(Mesh *mesh, TVertex v0, TVertex v1, TVertex v2)
{
	AddVertex(mesh, v0);
	AddVertex(mesh, v1);
	AddVertex(mesh, v2);
};

template<typename TIndex>
void AddIndex(Mesh *mesh, TIndex i)
{
	assert(mesh->indexCount != std::numeric_limits<TIndex>::max());
	((TIndex*)(mesh->indices))[mesh->indexCount++] = i;
};

template<typename TIndex>
void AddIndexTriangle(Mesh *mesh, TIndex i0, TIndex i1, TIndex i2)
{
	AddIndex(mesh, i0);
	AddIndex(mesh, i1);
	AddIndex(mesh, i2);
};

Mesh UtilMesh::MakeScreenQuad()
{
	Mesh mesh = {};

	struct Vertex
	{
		float position[3];
		float texCoords[2];
	};
	mesh.vertexAttributeSizes = vector<unsigned int>{POSITION_SIZE, TEXCOORD_SIZE};

	const Vertex vertices[] =
	{
		Vertex { {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },		// Bottom left
		Vertex { {1.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },		// Bottom right
		Vertex { {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },		// Top right
		Vertex { {-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },		// Top left
	};

	uint16_t indices[] =
	{
		0, 1, 2,
		2, 3, 0,
	};

	mesh.vertexCount = 4;
	mesh.vertexStride = sizeof(vertices) / mesh.vertexCount;
	mesh.vertices = malloc(mesh.vertexStride * mesh.vertexCount);
	mesh.indexCount = ARRAYSIZE(indices);
	mesh.indexStride = sizeof(uint16_t);
	mesh.indices = malloc(mesh.indexStride * mesh.indexCount);

	memcpy(mesh.vertices, vertices, sizeof(vertices));
	memcpy(mesh.indices, indices, sizeof(indices));

	return mesh;
}

Mesh UtilMesh::MakeFloor(float size)
{
	Mesh mesh = {};
	struct Vertex
	{
		float position[3];
		float normal[3];
		float texCoords[2];
	};
	mesh.vertexAttributeSizes = vector<unsigned int>{POSITION_SIZE, NORMAL_SIZE, TEXCOORD_SIZE};
	
	const Vertex vertices[] =
	{
		Vertex { {-size / 2.0f, 0.0f, size / 2.0f}, {0, 1, 0}, {0.0f, 0.0f} },		// Front left
		Vertex { {size / 2.0f, 0.0f, size / 2.0f}, {0, 1, 0}, {1.0f, 0.0f} },		// Front right
		Vertex { {-size/2.0f, 0.0f, -size/2.0f}, {0, 1, 0}, {0.0f, 1.0f} },			// Back left
		Vertex { {size/2.0f, 0.0f, -size/2.0f}, {0, 1, 0}, { 1.0f, 1.0f } },		// Back right
	};
	
	uint16_t indices[] =
	{
		0, 3, 2,
		1, 3, 0
	};
	
	mesh.vertexCount = ARRAYSIZE(vertices);
	mesh.vertexStride = sizeof(vertices) / mesh.vertexCount;
	mesh.vertices = malloc(mesh.vertexStride * mesh.vertexCount);
	mesh.indexCount = ARRAYSIZE(indices);
	mesh.indexStride = sizeof(uint16_t);
	mesh.indices = malloc(mesh.indexStride * mesh.indexCount);

	memcpy(mesh.vertices, vertices, sizeof(vertices));
	memcpy(mesh.indices, indices, sizeof(indices));
	return mesh;
}

Mesh UtilMesh::MakeCubeCenteredWithNormals(float edgeSize)
{
	struct Vertex
	{
		float position[3];
		float normal[3];
	};
	Mesh mesh = {};
	mesh.vertexAttributeSizes = vector<unsigned int>{POSITION_SIZE, NORMAL_SIZE};

	mesh.vertexCount = 36;
	mesh.vertexStride = sizeof(Vertex);
	mesh.vertices = malloc(mesh.vertexStride * mesh.vertexCount);
	mesh.indexCount = 36;
	mesh.indexStride = sizeof(uint16_t);
	mesh.indices = malloc(mesh.indexStride * mesh.indexCount);
	
	float size = edgeSize;
	Vertex cubeVertices[] =
	{
		// Front
		Vertex{ {-size / 2, -size / 2, size / 2}, {0, 0, 1} },	// LBN,
		Vertex{ {size / 2, -size / 2, size / 2}, {0, 0, 1} }, 	// RBN, 
		Vertex{ {size / 2, size / 2, size / 2}, {0, 0, 1} }, 	// RTN,
		Vertex{ {size / 2, size / 2, size / 2}, {0, 0, 1} }, 	// RTN, 
		Vertex{ {-size / 2, size / 2, size / 2}, {0, 0, 1} },	// LTN, 
		Vertex{ {-size / 2, -size / 2, size / 2}, {0, 0, 1} },	// LBN,

		// Top
		Vertex{ {-size / 2, size / 2, size / 2}, {0, 1, 0} },	// LTN, 
		Vertex{ {size / 2, size / 2, size / 2}, {0, 1, 0} }, 	// RTN, 
		Vertex{ {size / 2, size / 2, -size / 2}, {0, 1, 0} },	// RTF,
		Vertex{ {size / 2, size / 2, -size / 2}, {0, 1, 0} },	// RTF, 
		Vertex{ {-size / 2, size / 2, -size / 2}, {0, 1, 0} },	// LTF, 
		Vertex{ {-size / 2, size / 2, size / 2}, {0, 1, 0} },	// LTN,

		// Back
		Vertex{ {size / 2, -size / 2, -size / 2}, {0, 0, -1} }, // RBF, 
		Vertex{ {-size / 2, -size / 2, -size / 2}, {0, 0, -1} },// LBF, 
		Vertex{ {-size / 2, size / 2, -size / 2}, {0, 0, -1} },	// LTF,
		Vertex{ {-size / 2, size / 2, -size / 2}, {0, 0, -1} },	// LTF, 
		Vertex{ {size / 2, size / 2, -size / 2}, {0, 0, -1} },	// RTF, 
		Vertex{ {size / 2, -size / 2, -size / 2}, {0, 0, -1} }, // RBF,

		// Bottom
		Vertex{ {-size / 2, -size / 2, -size / 2}, {0, -1, 0} },// LBF, 
		Vertex{ {size / 2, -size / 2, -size / 2}, {0, -1, 0} }, // RBF, 
		Vertex{ {size / 2, -size / 2, size / 2}, {0, -1, 0} }, 	// RBN,
		Vertex{ {size / 2, -size / 2, size / 2}, {0, -1, 0} }, 	// RBN, 
		Vertex{ {-size / 2, -size / 2, size / 2}, {0, -1, 0} },	// LBN, 
		Vertex{ {-size / 2, -size / 2, -size / 2}, {0, -1, 0} },// LBF,

		// Right
		Vertex{ {size / 2, -size / 2, size / 2}, {1, 0, 0} }, 	// RBN, 
		Vertex{ {size / 2, -size / 2, -size / 2}, {1, 0, 0} }, 	// RBF, 
		Vertex{ {size / 2, size / 2, -size / 2}, {1, 0, 0} },	// RTF,
		Vertex{ {size / 2, size / 2, -size / 2}, {1, 0, 0} },	// RTF, 
		Vertex{ {size / 2, size / 2, size / 2}, {1, 0, 0} }, 	// RTN, 
		Vertex{ {size / 2, -size / 2, size / 2}, {1, 0, 0} }, 	// RBN,

		// Left
		Vertex{ {-size / 2, -size / 2, -size / 2}, {-1, 0, 0} },// LBF, 
		Vertex{ {-size / 2, -size / 2, size / 2}, {-1, 0, 0} },	// LBN, 
		Vertex{ {-size / 2, size / 2, size / 2}, {-1, 0, 0} },	// LTN,
		Vertex{ {-size / 2, size / 2, size / 2}, {-1, 0, 0} },	// LTN, 
		Vertex{ {-size / 2, size / 2, -size / 2}, {-1, 0, 0} },	// LTF, 
		Vertex{ {-size / 2, -size / 2, -size / 2}, {-1, 0, 0} }	// LBF,
	};

	memcpy(mesh.vertices, cubeVertices, sizeof(cubeVertices));

	uint16_t *indices = (uint16_t *)mesh.indices;
	for (unsigned int i = 0; i < mesh.indexCount; i++)
		indices[i] = uint16_t(i);

	return mesh;
}

Mesh UtilMesh::MakeSkyBox()
{
	struct SkyBoxVertex
	{
		float position[3];
	};

	SkyBoxVertex skyboxVertices[] = {
		// positions          
		SkyBoxVertex { -1.0f,  1.0f, -1.0f, },
		SkyBoxVertex { -1.0f, -1.0f, -1.0f, },
		SkyBoxVertex { 1.0f, -1.0f, -1.0f, },
		SkyBoxVertex { 1.0f, -1.0f, -1.0f },
		SkyBoxVertex { 1.0f,  1.0f, -1.0f, },
		SkyBoxVertex { -1.0f,  1.0f, -1.0f, },

		SkyBoxVertex { -1.0f, -1.0f,  1.0f, },
		SkyBoxVertex { -1.0f, -1.0f, -1.0f,	},
		SkyBoxVertex { -1.0f,  1.0f, -1.0f,	},
		SkyBoxVertex { -1.0f,  1.0f, -1.0f,	},
		SkyBoxVertex { -1.0f,  1.0f,  1.0f,	},
		SkyBoxVertex { -1.0f, -1.0f,  1.0f,	},

		SkyBoxVertex{ 1.0f, -1.0f, -1.0f, },
		SkyBoxVertex{ 1.0f, -1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f, -1.0f, },
		SkyBoxVertex{ 1.0f, -1.0f, -1.0f, },

		SkyBoxVertex{ -1.0f, -1.0f,  1.0f, },
		SkyBoxVertex{ -1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f, -1.0f,  1.0f, },
		SkyBoxVertex{ -1.0f, -1.0f,  1.0f, },

		SkyBoxVertex{ -1.0f,  1.0f, -1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f, -1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ -1.0f,  1.0f,  1.0f, },
		SkyBoxVertex{ -1.0f,  1.0f, -1.0f, },

		SkyBoxVertex{ -1.0f, -1.0f, -1.0f, },
		SkyBoxVertex{ -1.0f, -1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f, -1.0f, -1.0f, },
		SkyBoxVertex{ 1.0f, -1.0f, -1.0f, },
		SkyBoxVertex{ -1.0f, -1.0f,  1.0f, },
		SkyBoxVertex{ 1.0f, -1.0f,  1.0f },
	};



	Mesh mesh = {};
	mesh.vertexAttributeSizes = vector<unsigned int>{ POSITION_SIZE };

	mesh.vertexCount = ARRAYSIZE(skyboxVertices);
	mesh.vertexStride = sizeof(SkyBoxVertex);
	mesh.vertices = malloc(mesh.vertexStride * mesh.vertexCount);
	mesh.indexCount = mesh.vertexCount;
	mesh.indexStride = sizeof(uint16_t);
	mesh.indices = malloc(mesh.indexStride * mesh.indexCount);

	memcpy(mesh.vertices, skyboxVertices, sizeof(skyboxVertices));

	uint16_t *indices = (uint16_t *)mesh.indices;
	for (unsigned int i = 0; i < mesh.indexCount; i++)
		indices[i] = uint16_t(i);

	return mesh;
}

Mesh UtilMesh::DEBUGVector(vec3 startPoint, vec3 endPoint, vec3 color)
{
	struct DebugVertex
	{
		vec3 position;
		vec3 color;
	};
	Mesh mesh = {};
	mesh.vertexAttributeSizes = vector<unsigned int>{POSITION_SIZE, COLOR_SIZE};

	uint16_t numSides = 20;
	float width = 0.01f;
	float arrowHeadLenght = 0.2f;
	vec3 z = normalize(endPoint - startPoint);
	float length = glm::max(0.1f, glm::length(endPoint - startPoint) - arrowHeadLenght);
	endPoint = startPoint + z * length;

	mesh.vertexCount = (unsigned int)(2 * numSides + numSides + 1);
	mesh.vertexStride = sizeof(DebugVertex);
	mesh.vertices = malloc(mesh.vertexStride * mesh.vertexCount);
	mesh.indexCount = (unsigned int)(6 * numSides + 3 * numSides);
	mesh.indexStride = sizeof(uint16_t);
	mesh.indices = malloc(mesh.indexStride * mesh.indexCount);

	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	if (abs(dot(z, up)) > 0.95f)
	{
		up = vec3(0.0f, 0.0f, 1.0f);
	}
	vec3 x = normalize(cross(z, up));
	vec3 y = normalize(cross(x, z));

	float angle = 2 * float(PI) / numSides;
	mat4 rotation = glm::rotate(angle, z);
	vec3 offset = width * x;
	
	// Make top and bottom vertices
	DebugVertex *vertices = (DebugVertex*) mesh.vertices;
	unsigned int vertexCount = 0;
	for (unsigned int i = 0; i < numSides; ++i)
	{
		vertices[i].position = startPoint + offset;				// Bottom vertex
		vertices[i].color = color;				
		vertices[i + numSides].position = endPoint + offset;	// Top vertex
		vertices[i + numSides].color = color;
		offset = rotation * glm::vec4(offset, 0.0);
		vertexCount += 2;
	}

	// Make faces
	uint16_t *indices = (uint16_t*) mesh.indices;
	unsigned int indexCount = 0;
	for (uint16_t i = 0; i < numSides; ++i)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = (i + 1) % numSides;
		indices[indexCount++] = i + numSides;

		indices[indexCount++] = (i + 1) % numSides;
		uint16_t index = (i + numSides + 1);
		indices[indexCount++] = index == (2 * numSides) ? numSides : index;
		indices[indexCount++] = i + numSides;
	}

	// Make arrowhead
	uint16_t arrowVerticesOffset = uint16_t(vertexCount);
	offset = 4 * width * x;
	for (unsigned int i = 0; i < numSides; ++i)
	{
		vertices[vertexCount].position = endPoint + offset;
		vertices[vertexCount++].color = color;
		offset = rotation * glm::vec4(offset, 0.0);
	}
	vertices[vertexCount].position = endPoint + z * arrowHeadLenght;
	vertices[vertexCount++].color = color;
	
	uint16_t arrowTopIndex = uint16_t(vertexCount - 1);
	for (uint16_t i = 0; i < numSides; ++i)
	{
		indices[indexCount++] = arrowVerticesOffset + i;
		uint16_t firstIndexOffset = (i + 1 < numSides) ? i + 1 : 0;
		indices[indexCount++] = arrowVerticesOffset + firstIndexOffset;
		indices[indexCount++] = arrowTopIndex;
	}

	return mesh;
}

Mesh UtilMesh::DEBUGWorldAxes(float axLength)
{
	vec3 origin = vec3(0, 0, 0);
	vec3 x = vec3(1.0f, 0.0f, 0.0f);
	vec3 y = vec3(0.0f, 1.0f, 0.0f);
	vec3 z = vec3(0.0f, 0.0f, 1.0f);
	Mesh xMesh = UtilMesh::DEBUGVector(origin, axLength * x, x);
	Mesh yMesh = UtilMesh::DEBUGVector(origin, axLength * y, y);
	Mesh zMesh = UtilMesh::DEBUGVector(origin, axLength * z, z);
	
	Mesh axesMesh = ConcatenateMeshes(std::vector<Mesh> {xMesh, yMesh, zMesh});
	return axesMesh;
}

Mesh UtilMesh::DEBUGMakeCube(float edgeSize, vec3 color)
{
	struct Vertex
	{
		float position[3];
		vec3 color;
	};
	Mesh mesh = {};
	mesh.vertexAttributeSizes = vector<unsigned int>{ POSITION_SIZE, COLOR_SIZE };

	mesh.vertexCount = 36;
	mesh.vertexStride = sizeof(Vertex);
	mesh.vertices = malloc(mesh.vertexStride * mesh.vertexCount);
	mesh.indexCount = 36;
	mesh.indexStride = sizeof(uint16_t);
	mesh.indices = malloc(mesh.indexStride * mesh.indexCount);

	float size = edgeSize;
	Vertex cubeVertices[] =
	{
		// Front
		Vertex{ { -size / 2, -size / 2, size / 2 }, color },	// LBN,
		Vertex{ { size / 2, -size / 2, size / 2 }, color }, 	// RBN, 
		Vertex{ { size / 2, size / 2, size / 2 }, color }, 		// RTN,
		Vertex{ { size / 2, size / 2, size / 2 }, color },		// RTN, 
		Vertex{ { -size / 2, size / 2, size / 2 }, color },		// LTN, 
		Vertex{ { -size / 2, -size / 2, size / 2 }, color },	// LBN,

		// Top
		Vertex{ { -size / 2, size / 2, size / 2 }, color },		// LTN, 
		Vertex{ { size / 2, size / 2, size / 2 }, color },		// RTN, 
		Vertex{ { size / 2, size / 2, -size / 2 }, color },		// RTF,
		Vertex{ { size / 2, size / 2, -size / 2 }, color },		// RTF, 
		Vertex{ { -size / 2, size / 2, -size / 2 }, color },	// LTF, 
		Vertex{ { -size / 2, size / 2, size / 2 }, color },		// LTN,

		// Back
		Vertex{ { size / 2, -size / 2, -size / 2 }, color },	// RBF, 
		Vertex{ { -size / 2, -size / 2, -size / 2 }, color },	// LBF, 
		Vertex{ { -size / 2, size / 2, -size / 2 }, color },	// LTF,
		Vertex{ { -size / 2, size / 2, -size / 2 }, color },	// LTF, 
		Vertex{ { size / 2, size / 2, -size / 2 }, color },		// RTF, 
		Vertex{ { size / 2, -size / 2, -size / 2 }, color },	// RBF,

		// Bottom
		Vertex{ { -size / 2, -size / 2, -size / 2 }, color },	// LBF, 
		Vertex{ { size / 2, -size / 2, -size / 2 }, color },	// RBF, 
		Vertex{ { size / 2, -size / 2, size / 2 }, color }, 	// RBN,
		Vertex{ { size / 2, -size / 2, size / 2 }, color }, 	// RBN, 
		Vertex{ { -size / 2, -size / 2, size / 2 }, color },	// LBN, 
		Vertex{ { -size / 2, -size / 2, -size / 2 }, color },	// LBF,

		// Right
		Vertex{ { size / 2, -size / 2, size / 2 }, color }, 	// RBN, 
		Vertex{ { size / 2, -size / 2, -size / 2 }, color }, 	// RBF, 
		Vertex{ { size / 2, size / 2, -size / 2 }, color },		// RTF,
		Vertex{ { size / 2, size / 2, -size / 2 }, color },		// RTF, 
		Vertex{ { size / 2, size / 2, size / 2 }, color }, 		// RTN, 
		Vertex{ { size / 2, -size / 2, size / 2 }, color }, 	// RBN,

		// Left
		Vertex{ { -size / 2, -size / 2, -size / 2 }, color },	// LBF, 
		Vertex{ { -size / 2, -size / 2, size / 2 }, color },	// LBN, 
		Vertex{ { -size / 2, size / 2, size / 2 }, color },		// LTN,
		Vertex{ { -size / 2, size / 2, size / 2 }, color },		// LTN, 
		Vertex{ { -size / 2, size / 2, -size / 2 }, color },	// LTF, 
		Vertex{ { -size / 2, -size / 2, -size / 2 }, color }	// LBF,
	};

	memcpy(mesh.vertices, cubeVertices, sizeof(cubeVertices));

	uint16_t *indices = (uint16_t *)mesh.indices;
	for (unsigned int i = 0; i < mesh.indexCount; i++)
		indices[i] = uint16_t(i);

	return mesh;
}

Mesh ConcatenateMeshes(std::vector<Mesh> meshes)
{
	// Check that layout is identical
	vector<unsigned int> attributeSizes = meshes[0].vertexAttributeSizes;
	size_t vertexStride = meshes[0].vertexStride;
	size_t indexStride = meshes[0].indexStride;
	for (unsigned int i = 1; i < meshes.size(); ++i)
	{
		assert(meshes[i].vertexAttributeSizes == attributeSizes);
		assert(meshes[i].indexStride == indexStride);
		assert(meshes[i].vertexStride == vertexStride);
	}

	uint32_t totalIndexCount = 0;
	uint32_t totalVertexCount = 0;
	for (unsigned int i = 0; i < meshes.size(); ++i)
	{
		totalIndexCount += meshes[i].indexCount;
		totalVertexCount += meshes[i].vertexCount;
	}
	
	Mesh concat = {};
	concat.vertexCount = totalVertexCount;
	concat.vertexStride = vertexStride;
	concat.vertices = malloc(concat.vertexStride * concat.vertexCount);
	concat.indexCount = totalIndexCount;
	concat.indexStride = indexStride;
	concat.indices = malloc(concat.indexStride * concat.indexCount);
	concat.vertexAttributeSizes = attributeSizes;

	uint8_t *vertexPtrOffset = (uint8_t *)concat.vertices;
	uint8_t  *indexPtrOffset = (uint8_t *)concat.indices;

	unsigned int vertexOffset = 0;
	for (unsigned int i = 0; i < meshes.size(); ++i)
	{
		// Shift indices
		assert(indexStride == sizeof(uint16_t));	// Hard coded for 16b
		for (unsigned int j = 0; j < meshes[i].indexCount; ++j)
			((uint16_t*)(meshes[i].indices))[j] += uint16_t(vertexOffset);

		size_t dataSize = meshes[i].vertexCount * meshes[i].vertexStride;
		memcpy((void*)vertexPtrOffset, meshes[i].vertices, dataSize);
		vertexPtrOffset += dataSize;
		vertexOffset += meshes[i].vertexCount;

		dataSize = meshes[i].indexCount * meshes[i].indexStride;
		memcpy((void*)indexPtrOffset, meshes[i].indices, dataSize);
		indexPtrOffset += dataSize;
		UtilMesh::Free(meshes[i]);
	}
	return concat;
}

// polarAngle = latitude, azimuth = longitude.
glm::vec3 SphericalToCartesian(float r, float polarAngle, float azimuth)
{
	glm::vec3 cartesian;
	cartesian.x = r * sin(azimuth) * sin(polarAngle);
	cartesian.y = r * cos(polarAngle);
	cartesian.z = r * cos(azimuth) * sin(polarAngle);
	return cartesian;
}

Mesh UtilMesh::MakeUVSphere(unsigned int subdivisions, float radius)
{
	struct UVSphereVertex
	{
		vec3 position;
		vec3 normal;
		vec2 texCoords;
	};

	unsigned int stacks = subdivisions;
	unsigned int slices = subdivisions;
	float r = radius;
	vec3 c = vec3(0.0f, 0.0f, 0.0f);

	Mesh mesh = {};
	mesh.vertexCount = 0;
	mesh.vertices = malloc(3 * (slices * 2 + ((stacks - 2) * slices * 2)) * sizeof(UVSphereVertex));
	mesh.vertexStride = sizeof(UVSphereVertex);
	mesh.vertexAttributeSizes = vector<unsigned int>{ POSITION_SIZE, NORMAL_SIZE, TEXCOORD_SIZE };

	for (unsigned int stack = 0; stack < stacks; ++stack)
	{
		float polar1 = (float(stack) / stacks) * PI;
		float polar2 = (float(stack + 1) / stacks) * PI;

		for (unsigned int slice = 0; slice < slices; ++slice)
		{
			float azimuth1 = (float(slice) / slices) * 2 * PI;
			float azimuth2 = (float(slice + 1) / slices) * 2 * PI;

			UVSphereVertex v1 = {}, v2 = {}, v3 = {}, v4 = {};

			/* azim1      azim2
				 v1 ----- v4		polar1
 				 |	       |
				 v2 ----- v3		polar2
			
			*/
			v1.position = SphericalToCartesian(r, polar1, azimuth1);
			v2.position = SphericalToCartesian(r, polar2, azimuth1);
			v3.position = SphericalToCartesian(r, polar2, azimuth2);
			v4.position = SphericalToCartesian(r, polar1, azimuth2);

			v1.normal = normalize(vec3(v1.position) - c);
			v2.normal = normalize(vec3(v2.position) - c);
			v3.normal = normalize(vec3(v3.position) - c);
			v4.normal = normalize(vec3(v4.position) - c);

			float texAzimuth1 = float(slice) / float(slices + 1);
			float texAzimuth2 = float(slice + 1) / float(slices + 1);
			float texPolar1 = float(stack) / float(stacks + 1);
			float texPolar2 = float(stack + 1) / float(stacks + 1);

			v1.texCoords = vec2(texAzimuth1, texPolar1);
			v2.texCoords = vec2(texAzimuth1, texPolar2);
			v3.texCoords = vec2(texAzimuth2, texPolar2);
			v4.texCoords = vec2(texAzimuth2, texPolar1);

			if (stack == 0)									// First stack
				AddVertexTriangle(&mesh, v1, v2, v3);
			else if (stack == stacks - 1)
				AddVertexTriangle(&mesh, v2, v4, v1);	// Last stack
			else
			{
				AddVertexTriangle(&mesh, v1, v2, v3);	// Middle stacks
				AddVertexTriangle(&mesh, v3, v4, v1);
			}
		}
	}

	mesh.indexCount = mesh.vertexCount;
	mesh.indexStride = sizeof(uint32_t);
	mesh.indices = (uint32_t*)malloc(mesh.indexCount * mesh.indexStride);

	for (unsigned int i = 0; i < mesh.indexCount; ++i)
	{
		((uint32_t*)(mesh.indices))[i] = i;
	}

	return mesh;
}

struct IcoSphereVertex
{
	vec3 position;
	vec3 normal;
};

void AddIcosahedronUnitVertex(Mesh *mesh, IcoSphereVertex v)
{
	v.position = glm::normalize(v.position);
	AddVertex<IcoSphereVertex>(mesh, v);
}

// Based on http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
Mesh UtilMesh::MakeIcosahedronSphere(unsigned int recursionLevel)
{
	Mesh mesh = {};
	mesh.vertexCount = 0;
	mesh.vertexStride = sizeof(IcoSphereVertex);
	unsigned int finalVertexCount = 20 * 3 * (unsigned int)(powf(4.0f, float(recursionLevel)));
	mesh.vertices = malloc(finalVertexCount * mesh.vertexStride);
	mesh.vertexAttributeSizes = vector<unsigned int>{ POSITION_SIZE, NORMAL_SIZE };

	mesh.indexCount = 0;
	mesh.indexStride = sizeof(uint16_t);
	unsigned int finalIndexCount = finalVertexCount;
	mesh.indices = malloc(finalIndexCount * mesh.indexStride);

	// Create Icosahedron
	float t = (1.0f + sqrtf(5.0f)) / 2.0f;

	// Base icosahedron vertices
	IcoSphereVertex b[12] =
	{
		 IcoSphereVertex{ vec3(-1, t, 0), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(1, t, 0), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(-1, -t, 0), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(1, -t, 0), vec3(0, 0, 0) },

		 IcoSphereVertex{ vec3(0, -1, t), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(0, 1, t), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(0, -1, -t), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(0, 1, -t), vec3(0, 0, 0) },

		 IcoSphereVertex{ vec3(t, 0, -1), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(t, 0, 1), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(-t, 0, -1), vec3(0, 0, 0) },
		 IcoSphereVertex{ vec3(-t, 0, 1), vec3(0, 0, 0) },
	};

	AddVertexTriangle(&mesh, b[0], b[11], b[5]);
	AddVertexTriangle(&mesh, b[0], b[5], b[1]);
	AddVertexTriangle(&mesh, b[0], b[1], b[7]);
	AddVertexTriangle(&mesh, b[0], b[7], b[10]);
	AddVertexTriangle(&mesh, b[0], b[10], b[11]);

	AddVertexTriangle(&mesh, b[1], b[5], b[9]);
	AddVertexTriangle(&mesh, b[5], b[11], b[4]);
	AddVertexTriangle(&mesh, b[11], b[10], b[2]);
	AddVertexTriangle(&mesh, b[10], b[7], b[6]);
	AddVertexTriangle(&mesh, b[7], b[1], b[8]);

	AddVertexTriangle(&mesh, b[3], b[9], b[4]);
	AddVertexTriangle(&mesh, b[3], b[4], b[2]);
	AddVertexTriangle(&mesh, b[3], b[2], b[6]);
	AddVertexTriangle(&mesh, b[3], b[6], b[8]);
	AddVertexTriangle(&mesh, b[3], b[8], b[9]);

	AddVertexTriangle(&mesh, b[4], b[9], b[5]);
	AddVertexTriangle(&mesh, b[2], b[4], b[11]);
	AddVertexTriangle(&mesh, b[6], b[2], b[10]);
	AddVertexTriangle(&mesh, b[8], b[6], b[7]);
	AddVertexTriangle(&mesh, b[9], b[8], b[1]);

	// Subdivision
	for (unsigned int level = 0; level < recursionLevel; ++level)
	{
		std::vector<IcoSphereVertex> newVertices;
		for (unsigned int i = 0; i < mesh.vertexCount; i += 3)
		{
			IcoSphereVertex &v0 = ((IcoSphereVertex*)(mesh.vertices))[i];
			IcoSphereVertex &v1 = ((IcoSphereVertex*)(mesh.vertices))[i + 1];
			IcoSphereVertex &v2 = ((IcoSphereVertex*)(mesh.vertices))[i + 2];

			/*
				  V2
			      /\
			     /  \
			 V0 /____\ V1
			*/

			// Normalize to place the vertices on the unit sphere
			v0.position = glm::normalize(v0.position);
			v1.position = glm::normalize(v1.position);
			v2.position = glm::normalize(v2.position);

			IcoSphereVertex v0_v1;
			v0_v1.position = glm::normalize((v0.position + v1.position) / 2.0f);
			IcoSphereVertex v1_v2;
			v1_v2.position = glm::normalize((v1.position + v2.position) / 2.0f);
			IcoSphereVertex v2_v0;
			v2_v0.position = glm::normalize((v2.position + v0.position) / 2.0f);

			newVertices.push_back(v0);
			newVertices.push_back(v0_v1);
			newVertices.push_back(v2_v0);

			newVertices.push_back(v0_v1);
			newVertices.push_back(v1);
			newVertices.push_back(v1_v2);

			newVertices.push_back(v2_v0);
			newVertices.push_back(v1_v2);
			newVertices.push_back(v2);

			newVertices.push_back(v0_v1);
			newVertices.push_back(v1_v2);
			newVertices.push_back(v2_v0);
		}

		memcpy(mesh.vertices, newVertices.data(), newVertices.size() * sizeof(IcoSphereVertex));
		mesh.vertexCount = newVertices.size();
	}

	for (unsigned int i = 0; i < mesh.vertexCount; i += 3)
	{
			IcoSphereVertex &v0 = ((IcoSphereVertex*)(mesh.vertices))[i];
			IcoSphereVertex &v1 = ((IcoSphereVertex*)(mesh.vertices))[i + 1];
			IcoSphereVertex &v2 = ((IcoSphereVertex*)(mesh.vertices))[i + 2];
		
			vec3 normal = glm::cross(v1.position - v0.position, v2.position - v0.position);
			normal = glm::normalize(normal);
		
			v0.normal = normal;
			v1.normal = normal;
			v2.normal = normal;
	}

	for (unsigned int i = 0; i < mesh.vertexCount; ++i)
	{
		AddIndex<uint16_t>(&mesh, uint16_t(i));
	}

	return mesh;
}

void UtilMesh::Free(Mesh mesh)
{
	free(mesh.vertices);
	mesh.vertices = nullptr;
	free(mesh.indices);
	mesh.indices = nullptr;
}
