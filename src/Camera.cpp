#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "Camera.h"
#include "Graphics.h"
#include "UserInput.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::normalize;
using glm::length;
using glm::cross;

void CameraControl::SetProjection(Camera *camera, glm::mat4 projection)
{
	camera->projectionMatrix = projection;
}

void CameraControl::SetView(Camera *camera, glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
	camera->position = position;
	camera->target = target;
	camera->up = up;
	camera->viewMatrix = glm::lookAt(position, target, up);
}

void CameraControl::UpdateCamera(Camera *cam, double deltaTime, UserInput *userInput)
{
	float dt = float(deltaTime);

	// Vertical shift
	float &verticalSpeed = cam->verticalSpeed;
	float verticalAcceleration = 0.9f;
	float verticalDecceleration = 0.92f;

	verticalSpeed *= verticalDecceleration;
	if (userInput->wPressed)
	{
		verticalSpeed += verticalAcceleration * dt;
	}
	if (userInput->sPressed)
	{
		verticalSpeed -= verticalAcceleration * dt;
	}

	cam->position.y += verticalSpeed;
	cam->target.y += verticalSpeed;

	// Rotation
	float relX = userInput->mouseRelX;
	float relY = userInput->mouseRelY;
	float scroll = userInput->mouseWheel;

	float &yawSpeed = cam->yawSpeed;		// rotate around Y(up) axis
	float &pitchSpeed = cam->pitchSpeed;	// rotate around X(right) axis

	float acceleration = 0.03f;
	float decceleration = 0.89f;
	yawSpeed = decceleration * yawSpeed + -relX * acceleration * dt;
	pitchSpeed = decceleration * pitchSpeed + -relY * acceleration * dt;

	vec3 camForward = normalize(cam->target - cam->position);
	vec3 camRight = normalize(cross(camForward, vec3(0.0f, 1.0f, 0.0f)));
	vec3 camUp = normalize(cross(camRight, camForward));

	// Do not allow the camera to get completely parallel with the positive/negative y axis
	pitchSpeed = camForward.y < -0.98f ? glm::max(0.0f, pitchSpeed) : pitchSpeed;
	pitchSpeed = camForward.y > 0.98f ? glm::min(0.0f, pitchSpeed) : pitchSpeed;

	float dist = length(cam->position - cam->target);
	vec4 toCamera = normalize(vec4((cam->position - cam->target), 0.0f));

	toCamera = glm::rotate(yawSpeed, vec3(0.0f, 1.0f, 0.0f)) * toCamera;
	toCamera = glm::rotate(pitchSpeed, camRight) * toCamera;
	cam->position = cam->target + dist * vec3(toCamera);

	// Recalculate up for the View Matrix
	camForward = cam->target - cam->position;
	camRight = normalize(cross(camForward, vec3(0.0f, 1.0f, 0.0f)));
	camUp = normalize(cross(camRight, camForward));

	// Zoom
	float &zoom = cam->zoomAmount;
	vec3 forward = cam->target - cam->position;
	zoom = 0.89f * zoom + (scroll * 0.01f);
	if (zoom > 0 && glm::length(forward) > 0.f || zoom < 0 && glm::length(forward) < 90.0f)
	{
		cam->position += forward * zoom;
	}
	else
	{
		zoom = 0.0f;
	}

	cam->viewMatrix = glm::lookAt(cam->position, cam->target, camUp);
}

// The correct program has to be set prior to calling this function.
void CameraControl::Use(Camera *camera, unsigned int program)
{
	Graphics::SetMatrixUniform(program, camera->projectionMatrix, PROJECTION_UNIFORM_NAME);
	Graphics::SetMatrixUniform(program, camera->viewMatrix, VIEW_UNIFORM_NAME);
}