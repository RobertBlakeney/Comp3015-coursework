#version 460

layout (location = 0) out vec4 fragColour;
uniform vec4 colour;

void main() {
	fragColour = colour;
}