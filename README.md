# Cw1 OpenGL Scene

## Tools used

* Visual Studio 2022
* Windows 11
* glad, glfw, glm

## Code Explanation

The code is ran from a main.cpp file with scenebasic_uniform.cpp having the enviroment to execute the code. This is split into a couple functions. init for setting up the scene, comile to compile and link shaders, update for any realtime updates such as positional or rotation data and a render loop for seting models on the scene and rendering them with textures. 

Scenebasic_uniform.h has dependencies, function definitions and loaded textures. Add these here.

The shaders folder is used for storing vertex and fragment shaders. Two sets currently exist for the scene and the skybox. any new shaders should be added here and the scenebasic uniform shaders should be modified for scene changes.

media folder is for obj and texture files needed.

## Demo of scene

[video][https://www.youtube.com/watch?v=KauvRVzMQLs]

