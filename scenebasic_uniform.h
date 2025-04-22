#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "helper/glslprogram.h"
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/texture.h"
#include "helper/skybox.h"
#include "helper/grid.h"
#include "helper/random.h"
#include "helper/particleutils.h"

void mouse_callback(GLFWwindow* window, double xPos, double yPos);

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog, skyProg, particleProg, flatProg;

    Random random;
    GLuint initVel, startTime, particles, nParticles;
    Grid grid;

    GLuint tex = Texture::loadTexture("media/peng.png");
    GLuint dirtTex = Texture::loadTexture("media/dirt.png");
    GLuint grey = Texture::loadTexture("media/grey.jpg");
    GLuint fishTex = Texture::loadTexture("media/fish.png");

    GLuint cubeTex = Texture::loadHdrCubeMap("media/sky/sky");
    
    GLuint fsQuad, renderTex, intermediateTex, renderFBO, intermediateFBO;

    SkyBox sky;
    Plane plane;
    
    std::unique_ptr<ObjMesh> mesh;
    std::unique_ptr<ObjMesh> fish;

    glm::vec3 emitterPos, emitterDir;
    
    float tPrev;
    float angle;
    float pAngle, pTime, particleLifetime;

    void initBuffers();
    float randFloat();

    void compile();
    void SetMatrices(GLSLProgram &p);

    void setupFBO();
    void pass1();
    void pass2();
    void pass3();
    float gauss(float, float);

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    
};

#endif // SCENEBASIC_UNIFORM_H
