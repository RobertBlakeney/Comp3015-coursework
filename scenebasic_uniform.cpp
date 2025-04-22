#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>
#include <stdlib.h>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f; 
float pitch = 0.0f;
bool firstEntry = true;

float cameraLastXPos = 800.0f / 2.0f;
float cameraLastYPos = 600.0f / 2.0f;

glm::vec3 penguinPosition = glm::vec3(0.0f, 1.2f, 0.0f);
float penguinRotation = 0.0f;
float penguinScale = 2.0f;

glm::vec3 fishPosition = glm::vec3(2.0f, 1.2f, 2.0f);

SceneBasic_Uniform::SceneBasic_Uniform() : pTime(200000), particleLifetime(5.5f), nParticles(100), 
											emitterPos(0, 5, 0), emitterDir(0, -2, 0),
	tPrev(0),
	plane(50.0f, 50.0f, 1, 1),
	sky(100.0f) {
	mesh = ObjMesh::load("media/peng.obj", true);
	fish = ObjMesh::load("media/fish.obj", true);
}

void SceneBasic_Uniform::initScene()
{
	glfwSetCursorPosCallback(glfwGetCurrentContext(), mouse_callback);

    compile();
	//glClearColor(1.0f, 1.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
	view = glm::lookAt(vec3(7.0 * cos(angle), 4.0, 7.0 * sin(angle)), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);

	model = mat4(1.0f);

	angle = 0.0f;

	setupFBO();

	GLfloat verts[] = {
		-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0, 1.0f, 0.0f
	};

	GLfloat tc[] = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	unsigned int handle[2];
	glGenBuffers(2, handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dirtTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);

	glGenVertexArrays(1, &fsQuad);
	glBindVertexArray(fsQuad);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	prog.setUniform("Light.L", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Light.La", vec3(0.5f));
	prog.setUniform("Light.Exponent", vec3(50.0f));
	prog.setUniform("Light.Cutoff", glm::radians(15.0f));
	//prog.setUniform("Fog.MaxDist", 15.0f);
	//prog.setUniform("Fog.MinDist", 1.0f);
	//prog.setUniform("Fog.Colour", vec3(0.5f, 0.5f, 0.5f));

	float weights[5], sum, sigma2 = 10.0f;

	weights[0] = gauss(0, sigma2);
	sum = weights[0];
	for (int i = 1; i < 5; i++) {
		weights[i] = gauss(float(i), sigma2);
		sum += 2 * weights[i];
	}


	for (int i = 1; i < 5; i++) {
		std::stringstream uniName;
		uniName << "weight[" << i << "]";

		float val = weights[i] / sum;
		prog.setUniform(uniName.str().c_str(), val);
	}

	particleProg.use();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	pAngle = glm::half_pi<float>();
	//pAngle = 0;
	initBuffers();
	
	particleProg.setUniform("particleTex", 0);
	particleProg.setUniform("particleLifetime", particleLifetime);
	particleProg.setUniform("particleSize", 0.005f);
	particleProg.setUniform("gravity", vec3(0.0f, -0.002f, 0.0f));
	particleProg.setUniform("emitterPos", emitterPos);

	flatProg.use();
	flatProg.setUniform("colour", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

	prog.use();
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
			
		skyProg.compileShader("shader/skybox.vert");
		skyProg.compileShader("shader/skybox.frag");
		skyProg.link();

		particleProg.compileShader("shader/particle.vert");
		particleProg.compileShader("shader/particle.frag");
		particleProg.link();

		flatProg.compileShader("shader/flat.vert");
		flatProg.compileShader("shader/flat.frag");
		flatProg.link();
		
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	pTime = t;
	pAngle = std::fmod(angle + 0.01f, glm::two_pi<float>());


	float deltaT = t - tPrev;
	tPrev = t;
	angle += 0.25f * deltaT;
	if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();

	float fishDistance = abs(fishPosition.x - penguinPosition.x) + abs(fishPosition.z - penguinPosition.z);

	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_E) == GLFW_PRESS && fishDistance < 2.5f) {
		fishPosition = glm::vec3(rand() % (10 - (-10) + 1), 1.2f, rand() % (10 - (-10) + 1));
		penguinScale += 0.1;
		penguinPosition.y += 0.055;
	}


	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT) == GLFW_PRESS) {
		penguinRotation += 3.0f * deltaT;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
		penguinRotation -= 3.0f * deltaT;
	}

	// Move penguin forward/backward with W/S keys
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_UP) == GLFW_PRESS) {
		penguinPosition.x += sin(penguinRotation) * 6.0f * deltaT;
		penguinPosition.z += cos(penguinRotation) * 6.0f * deltaT;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_DOWN) == GLFW_PRESS) {
		penguinPosition.x -= sin(penguinRotation) * 6.0f * deltaT;
		penguinPosition.z -= cos(penguinRotation) * 6.0f * deltaT;
	}
}


void SceneBasic_Uniform::render()
{
	pass1();
	pass2();
	pass3();
}

void SceneBasic_Uniform::pass1() {
	prog.setUniform("pass", 1);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float cameraSpeed = 0.2f;

	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	//view = glm::lookAt(vec3(7.0 * cos(angle), 4.0, 7.0 * sin(angle)), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);

	vec4 lightPos = vec4(10.0f * cos(angle), 80.0f, 10.0f * sin(angle), 1.0f);
	prog.setUniform("Light.Position", vec4(view * lightPos));
	mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
	prog.setUniform("Light.Direction", normalMatrix * vec3(-lightPos));


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dirtTex);

	prog.setUniform("Material.Kd", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Ka", vec3(0.1f, 0.1f, 0.1f));
	prog.setUniform("Material.Shininess", 100.0f);

	model = mat4(1.0f);
	model = glm::translate(model, penguinPosition);
	model = glm::rotate(model, penguinRotation, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, vec3(penguinScale));
	SetMatrices(prog);
	mesh->render();

	prog.setUniform("Material.Kd", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Ka", vec3(0.1f, 0.1f, 0.1f));
	prog.setUniform("Material.Shininess", 100.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fishTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fishTex);

	model = mat4(1.0f);
	model = glm::translate(model, fishPosition);
	model = glm::scale(model, vec3(0.1f));
	SetMatrices(prog);
	fish->render();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	prog.setUniform("Material.Kd", vec3(0.2f, 0.5f, 0.9f));
	prog.setUniform("Material.Ks", vec3(0.8f, 0.8f, 0.8f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.5f, 0.9f));
	prog.setUniform("Material.Shininess", 180.0f);

	model = mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f));
	SetMatrices(prog);
	plane.render();

	skyProg.use();
	model = mat4(1.0f);
	SetMatrices(skyProg);
	sky.render();

	prog.use();
}

void SceneBasic_Uniform::pass2() {
	prog.setUniform("pass", 2);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderTex);

	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	model = mat4(1.0f);
	view = mat4(1.0f);
	projection = mat4(1.0f);

	SetMatrices(prog);

	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindVertexArray(0);

}

void SceneBasic_Uniform::pass3() {
	prog.setUniform("pass", 3);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, intermediateTex);

	glClear(GL_COLOR_BUFFER_BIT);

	model = mat4(1.0f);
	view = mat4(1.0f);
	projection = mat4(1.0f);

	SetMatrices(prog);

	glBindVertexArray(fsQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindVertexArray(0);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grey);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grey);

	flatProg.use();
	SetMatrices(flatProg);
	grid.render();

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	particleProg.use();
	SetMatrices(particleProg);
	particleProg.setUniform("time", pTime);
	glBindVertexArray(particles);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);

	prog.use();
}

void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
    width = w;
    height = h;

	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);

}

void SceneBasic_Uniform::SetMatrices(GLSLProgram& p) {
	mat4 mv = view * model;
	p.setUniform("ModelViewMatrix", mv);
	p.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	p.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::setupFBO() {
	glGenFramebuffers(1, &renderFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

	GLuint depthBuf;

	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	//GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//if (result == GL_FRAMEBUFFER_COMPLETE) {
	//	std::cout << "Framebuffer complete." << endl;
	//}
	//else{
	//	std::cout << "Framebuffer error: " << result << endl;
	//}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

	glGenFramebuffers(1, &intermediateTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, intermediateTex);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTex, 0);

	glDrawBuffers(1, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float SceneBasic_Uniform::gauss(float x, float sigma2) {
	double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
	double exponent = -(x * x) / (2.0 * sigma2);

	return (float)(coeff * exp(exponent));
}

void SceneBasic_Uniform::initBuffers() {
	glGenBuffers(1, &initVel);
	glGenBuffers(1, &startTime);

	int size = nParticles * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, initVel);
	glBufferData(GL_ARRAY_BUFFER, size * 3, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, startTime);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

	glm::mat3 emitterBasis = ParticleUtils::makeArbitraryBasis(emitterDir);
	vec3 v(0.0f);
	float velocity, theta, phi;
	std::vector<GLfloat> data(nParticles * 3);
	for (uint32_t i = 0; i < nParticles; i++) {
		theta = glm::mix(0.0f, glm::pi<float>() / 20.0f, randFloat());
		phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());

		v.x = sinf(theta) * cosf(phi);
		v.y = cosf(theta);
		v.z = sinf(theta) * sinf(phi);

		velocity = glm::mix(1.25f, 1.5f, randFloat());
		v = glm::normalize(emitterBasis * v) * velocity;

		data[3 * i] = v.x;
		data[3 * i + 1] = v.y;
		data[3 * i + 2] = v.z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, initVel);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data());

	float rate = particleLifetime / nParticles;
	for (int i = 0; i < nParticles; i++) {
		data[i] = rate * i;
	}

	glBindBuffer(GL_ARRAY_BUFFER, startTime);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles *sizeof(float), data.data());

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &particles);
	glBindVertexArray(particles);
	glBindBuffer(GL_ARRAY_BUFFER, initVel);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, startTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glVertexAttribDivisor(0, 1);
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
}

float SceneBasic_Uniform::randFloat() {
	return random.nextFloat();
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstEntry) {
		cameraLastXPos = (float)xPos;
		cameraLastXPos = (float)yPos;
		firstEntry = false;
	}

	float xOffset = (float)xPos - cameraLastXPos;
	float yOffset = cameraLastYPos - (float)yPos;

	cameraLastXPos = (float)xPos;
	cameraLastYPos = (float)yPos;

	const float sensitivity = 0.025f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	else if (pitch < -89.0f)
		pitch = -89.0f;

	vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = normalize(direction);

}
