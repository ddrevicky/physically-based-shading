#version 330 core

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout (location = 0) in vec3 inPosition;

out vec3 vsLocalPosition;

void main()
{
    vsLocalPosition = inPosition;
    vec4 pos = uProjectionMatrix * mat4(mat3(uViewMatrix)) * vec4(inPosition, 1.0);	// Remove translation
	gl_Position = pos.xyww;															// All fragments will have depth value of 1.0 and fail the depth test for regular scene objects
}