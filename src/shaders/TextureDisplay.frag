#version 330 core

uniform sampler2D uTexSampler0;

in vec2 vsTexCoords;
out vec4 fragColor;
  
void main()
{
#if 0
    float depth = texture(uTexSampler0, vsTexCoords).r;
    fragColor = vec4(depth, depth, depth, 1.0);
#else
	vec3 color = texture(uTexSampler0, vsTexCoords).rgb;
	//vec3 color = vec3(texture(uTexSampler0, vsTexCoords).rg, 0);
    color = color / (color + vec3(1.0));		// HDR
	fragColor = vec4(color, 1.0);

#endif
} 