#version 330 core

uniform samplerCube uTexSkyBox;

in vec3 vsLocalPosition;

out vec4 fragColor;

const float gamma = 2.2;

void main()
{    
	vec3 cubemapTexCoords = normalize(vsLocalPosition);
	vec3 color = textureLod(uTexSkyBox, cubemapTexCoords, 0.0).rgb;
	color = color / (color + vec3(1.0));	// HDR
	color = pow(color, vec3(1.0 / gamma));
    fragColor = vec4(color, 1.0);
}