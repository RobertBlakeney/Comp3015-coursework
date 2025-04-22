#version 460

in float transp;
in vec2 texCoord;

layout (location = 0) out vec4 FragColour;

uniform sampler2D particleTex;

void main () {
	FragColour = texture(particleTex, texCoord);
	FragColour.a *= transp;

	if (FragColour.a < 0.01) discard;
}
