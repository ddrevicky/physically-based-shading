#version 330 core

uniform sampler2D uEquirectangularMap;

in vec3 vsLocalPosition;
out vec4 fragColor;

#define PI 3.14159265359

vec2 CartesianToSpherical(vec3 cartesian)
{
	float theta = atan(cartesian.z, cartesian.x);	// Azimuth angle (in the X-Z plane) in range [-PI, PI]
	float phi = acos(cartesian.y);					// Polar angle (measured from the top y-axis) [0 - for up pointing, PI - bottom]
	return vec2(theta, phi);
}

void main()
{
	vec2 spherical = CartesianToSpherical(normalize(vsLocalPosition));

	// Transform to [0, 1]
	float v = 1 - (spherical.y / PI);
	float u = spherical.x / (2.0*PI) + 0.5;

	vec3 color = texture(uEquirectangularMap, vec2(u, v)).rgb;
	fragColor = vec4(color, 1.0);
}