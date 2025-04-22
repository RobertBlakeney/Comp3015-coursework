#version 460

layout (location = 0) in vec3 VertexInitVel;
layout (location = 1) in float VertexBirthTime;

out float transp;
out vec2 texCoord;

uniform float time;
uniform vec3 gravity = vec3(0.0, -0.05, 0.0);
uniform float particleLifetime;
uniform float particleSize = 1.0;
uniform vec3 emitterPos;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;

const vec3 offsets[] = vec3[] (vec3(-3.5, -3.5, 0), vec3(3.5, -3.5, 0), vec3(3.5, 3.5, 0), vec3(-3.5, -3.5, 0), vec3(3.5, 3.5, 0), vec3(-3.5, 03.5, 0));

const vec2 texCoords[] = vec2[] (vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1));

void main() {
	vec3 cameraPos;
	float t = time - VertexBirthTime;
	if (t >= 0 && t < particleLifetime) {
		vec3 pos = emitterPos + VertexInitVel * t + gravity * t * t;

		cameraPos = (ModelViewMatrix * vec4(pos, 1)).xyz + (offsets[gl_VertexID] * particleSize);
		transp = mix(1, 0, t / particleLifetime);
	}
	else {
		cameraPos = vec3(0);
		transp = 0.0;
	}

	texCoord = texCoords[gl_VertexID];

	gl_Position = MVP * vec4(cameraPos, 1);
}