#version 330 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;

out vec2 vsTexCoords;

void main()
{
    gl_Position = vec4(inPosition, 1.0f);
    vsTexCoords = inTexCoords;
}