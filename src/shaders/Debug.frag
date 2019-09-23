#version 330 core

in vec3 vsColor;

out vec4 outColor;

void main() 
{
	outColor = vec4(vsColor, 1.0f);
}
