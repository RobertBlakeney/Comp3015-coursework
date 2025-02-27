#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

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

SceneBasic_Uniform::SceneBasic_Uniform() :
	tPrev(0),
	plane(50.0f, 50.0f, 1, 1),
	sky(100.0f) {
	mesh = ObjMesh::load("media/peng.obj", true);
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

	prog.setUniform("edgeThreshold", 0.05f);
	prog.setUniform("Light.L", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Light.La", vec3(0.5f));
	prog.setUniform("Light.Exponent", vec3(50.0f));
	prog.setUniform("Light.Cutoff", glm::radians(15.0f));
	prog.setUniform("Fog.MaxDist", 15.0f);
	prog.setUniform("Fog.MinDist", 1.0f);
	prog.setUniform("Fog.Colour", vec3(0.5f, 0.5f, 0.5f));
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		skyProg.compileShader("shader/skybox.vert");
		skyProg.compileShader("shader/skybox.frag");
		prog.link();
		skyProg.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	float deltaT = t - tPrev;
	tPrev = t;
	angle += 0.25f * deltaT;
	if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();
}

void SceneBasic_Uniform::render()
{
	pass1();
	glFlush();
	pass2();
}

void SceneBasic_Uniform::pass1() {
	prog.setUniform("pass", 1);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float cameraSpeed = 0.05f;

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

	vec4 lightPos = vec4(10.0f * cos(angle), 10.0f, 10.0f * sin(angle), 1.0f);
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
	model = glm::translate(model, vec3(0.0f, 1.2f, 0.0f));
	model = glm::scale(model, vec3(2.0f));
	SetMatrices(prog);
	mesh->render();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grey);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grey);

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.0f, 0.0f, 0.0f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 180.0f);

	model = mat4(1.0f);
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	glBindVertexArray(0);
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
	glGenFramebuffers(1, &fboHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

	GLuint depthBuf;

	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result == GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer complete." << endl;
	}
	else{
		std::cout << "Framebuffer error: " << result << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
