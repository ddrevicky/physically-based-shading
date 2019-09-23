#version 330 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

out vec3 vsColor;

void main()
{
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(inPosition, 1.0);
	vsColor = inColor;
}