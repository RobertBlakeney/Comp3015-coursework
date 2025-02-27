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

void mouse_callback(GLFWwindow* window, double xPos, double yPos);

class SceneBasic_Uniform : public Scene
{
private:
    
    GLSLProgram prog, skyProg;
    GLuint tex = Texture::loadTexture("media/peng.png");
    GLuint dirtTex = Texture::loadTexture("media/dirt.png");
    GLuint grey = Texture::loadTexture("media/grey.jpg");

    GLuint cubeTex = Texture::loadHdrCubeMap("media/sky/sky");
    
    GLuint fsQuad, renderTex, fboHandle;

    SkyBox sky;
    Plane plane;
    
    std::unique_ptr<ObjMesh> mesh;
    
    float tPrev;
    float angle;

    void compile();
    void SetMatrices(GLSLProgram &p);

    void setupFBO();
    void pass1();
    void pass2();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    
};

#endif // SCENEBASIC_UNIFORM_H
