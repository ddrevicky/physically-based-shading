#version 330 core

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout (location = 0) in vec3 inPosition;

out vec3 vsLocalPosition;

void main()
{
    vsLocalPosition = inPosition;
    vec4 pos = uProjectionMatrix * uViewMatrix* vec4(inPosition, 1.0);
	gl_Position = pos;													
}