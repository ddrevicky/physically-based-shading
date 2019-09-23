#pragma once

#include <glm/glm.hpp>

struct UserInput;

struct Camera
{
	glm::vec3 up;
	glm::vec3 position;
	glm::vec3 target;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	float yawSpeed = 0.0f;
	float pitchSpeed = 0.0f;
	float verticalSpeed = 0.0f;
	float zoomAmount = 0.0f;
};

namespace CameraControl
{
	void SetView(Camera *camera, glm::vec3 position, glm::vec3 target, glm::vec3 up);
	void SetProjection(Camera *camera, glm::mat4);
	void UpdateCamera(Camera *camera, double dt, UserInput *userInput);
	void Use(Camera *camera, unsigned int program);
}