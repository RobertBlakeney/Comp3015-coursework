#version 460


layout (binding = 0) uniform samplerCube SkyBoxTex;
in vec3 Vec;
layout (location = 0) out vec4 FragColour;

void main() {
	vec3 texColour = texture(SkyBoxTex, normalize(Vec)).rgb;
	texColour = pow(texColour, vec3(1.0/2.2));
	FragColour = vec4(texColour, 1.0);
}

