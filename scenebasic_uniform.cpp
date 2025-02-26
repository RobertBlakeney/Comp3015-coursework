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



SceneBasic_Uniform::SceneBasic_Uniform() :
	tPrev(0),
	plane(50.0f, 50.0f, 1, 1) {
	mesh = ObjMesh::load("media/crab.obj", true);
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);
    view = glm::lookAt(vec3(0.0f, 2.0f, 6.0f), vec3(0.0f, 1.0f, -2.0f), vec3(0.0f, 1.0f, 0.0f));

	model = mat4(1.0f);


	projection = mat4(1.0f);
	angle = 0.0f;

	prog.setUniform("Light.L", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Light.La", vec3(0.5f));
	prog.setUniform("Light.Exponent", vec3(50.0f));
	prog.setUniform("Light.Cutoff", glm::radians(15.0f));
	prog.setUniform("Fog.MaxDist", 30.0f);
	prog.setUniform("Fog.MinDist", 1.0f);
	prog.setUniform("Fog.Colour", vec3(0.5f, 0.5f, 0.5f));


	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, crabTex);
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	//float deltaT = t - tPrev;
	//tPrev = t;
	//angle += 0.25f *  deltaT;
	//if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();

}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	vec4 lightPos = vec4(0.0f, 10.0f, 0.0f, 1.0f);
	prog.setUniform("Light.Position", vec4(view * lightPos));
	mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
	prog.setUniform("Light.Direction", normalMatrix * vec3(-lightPos));

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, crabTex);

	prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f));
	prog.setUniform("Material.Shininess", 100.0f);


	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 0.55f, 0.0f));
	model = glm::scale(model, vec3(2.0f));
	SetMatrices();
	mesh->render();
	
	//glBindTexture(GL_TEXTURE_2D, 0);

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.0f, 0.0f, 0.0f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 180.0f);

	model = mat4(1.0f);
	SetMatrices();
	plane.render();


	
}




void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
    width = w;
    height = h;

	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);

}

void SceneBasic_Uniform::SetMatrices() {
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);
}
