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

    glEnable(GL_DEPTH_TEST);
    view = glm::lookAt(vec3(0.8f, 2.0f, 5.0f), vec3(0.0f, 1.0f, -2.0f), vec3(0.0f, 1.0f, 0.0f));

	model = mat4(1.0f);

	angle = 0.0f;
	projection = mat4(1.0f);

	prog.setUniform("Light.L", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Light.La", vec3(0.5f));
	prog.setUniform("Light.Exponent", vec3(50.0f));
	prog.setUniform("Light.Cutoff", glm::radians(15.0f));
	prog.setUniform("Fog.MaxDist", 15.0f);
	prog.setUniform("Fog.MinDist", 1.0f);
	prog.setUniform("Fog.Colour", vec3(0.5f, 0.5f, 0.5f));


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dirtTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
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

	vec4 lightPos = vec4(10.0f * cos(angle), 10.0f, 10.0f * sin(angle), 1.0f);
	prog.setUniform("Light.Position", vec4(view * lightPos));
	mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
	prog.setUniform("Light.Direction", normalMatrix * vec3(-lightPos));



	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dirtTex);


	prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f));
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
