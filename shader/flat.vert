#version 460

layout (location = 0) in vec2 vertexPosition;
uniform mat4 ModelViewMatrix;

void main() {
	gl_Position = ModelViewMatrix * vec4(vertexPosition, 0.0, 1.0);
}

