#version 330 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uLightViewProjectionMatrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

out VS_OUT	
{
	vec3 normal;
	vec2 texCoords;
	vec3 posWorld;
	vec4 posLightSpace;
} vs_out;

void main()
{
	vec4 posWorld = uModelMatrix * vec4(inPosition, 1.0f);
	vs_out.posWorld = vec3(posWorld);
	vs_out.posLightSpace = uLightViewProjectionMatrix * posWorld;
	gl_Position = uProjectionMatrix * uViewMatrix * posWorld;

	vs_out.normal = transpose(inverse(mat3(uModelMatrix))) * inNormal;
	vs_out.texCoords = inTexCoords;
}