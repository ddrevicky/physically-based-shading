#version 330 core

uniform samplerCube uHDREnvironmentCubemap;

in vec3 vsLocalPosition;
out vec4 fragColor;

#define PI 3.14159265359

void main()
{
	vec3 N = normalize(vsLocalPosition);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 left = normalize(cross(up, N));
	up = normalize(cross(N, left));

	const float riemannSumStep = 0.015;
	int numSamples = 0;
	vec3 irradiance = vec3(0.0);
	for (float polar = 0.0; polar < 0.5 * PI; polar += riemannSumStep)
	{
		for (float azimuth = 0.0; azimuth < 2.0 * PI; azimuth += riemannSumStep)
		{
			vec3 tangentDir = vec3(sin(polar) * cos(azimuth), sin(polar) * sin(azimuth), cos(polar));	
			vec3 sampleDir = tangentDir.x * left + tangentDir.y * up + tangentDir.z * N;
			irradiance += texture(uHDREnvironmentCubemap, sampleDir).rgb * cos(polar) * sin(polar);
			++numSamples;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(numSamples));
	fragColor = vec4(irradiance, 1.0);
}