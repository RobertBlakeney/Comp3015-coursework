# OpenGL Scene

## Tools used

* Visual Studio 2022
* Windows 11
* glad, glfw, glm

## Features implemented
### cw1

- Blinn phong lighting
- Texturing
- texture mixing
- spotlights
- skybox
- light animation
- Edge detection
- Keyboard and mouse controls

### cw2

- Gaussian blur
- particle fountain
- Alpha discard
- Fog
- Gamification of scene

## Code Explanation

### Use of program

The program can be run either through visual studio or by using the an exe from compiling the software. Using thhe exe will required the contents of the media folder and will also need opengl dependencies used in C:/users/pulbic/opengl. These are glad, glfw, glm.

Controlling the scene is possible with the use of the wasd keys and the mouse. This will control the camera. To move the penguin the up and sown arrow keys can be used for forward and backwards with left and right for rotation of the model. Pressing E when near a model fish will allow interaction with it.

### cw1

The code is ran from a main.cpp file with scenebasic_uniform.cpp having the enviroment to execute the code. This is split into a couple functions. init for setting up the scene, comile to compile and link shaders, update for any realtime updates such as positional or rotation data and a render loop for seting models on the scene and rendering them with textures. 

Scenebasic_uniform.h has dependencies, function definitions and loaded textures. Add these here.

The shaders folder is used for storing vertex and fragment shaders. Two sets currently exist for the scene and the skybox. any new shaders should be added here and the scenebasic uniform shaders should be modified for scene changes.

media folder is for obj and texture files needed.

### cw2 addtions

Gaussian blur - This adds a blur effect on the screen that is less prominent the closr the camera is. This was achieved through taking three passes of the scene. One for lighting, one to do vertical and one to do horizontal blur. The feature works out to have a strong effect on the scene. The freature achieves blur through convolution loops in the fragment shader and rendering it via preforming the blur on a 2D frame of the scene. 

Particle fountain - This feature is implemenented in a couple of new shader files. The effect achieves a series of instanced particles throughout the scene in a pretermined direction for a set amount of time. In this implmnetation the particle fountain is set up attached to the camera. This allows a snow like effect at the start of the scene.

Gameification - The gamification of the scene revolves around moving the penguin model around the scene to collect fish. The camera can be used to explore and find the fish which a player can navigate to. Collecting the fish with 'E' makes the penguin grow in size. The game goal is to make the penguin as large as possible. The chellenge in the game is introduced through the blur effect on the camera. This makes it harder to spot the fish which can make the player take longer to locate them.

Alpha discard and fog - These features are additions missed from cw1. They both added in the scenebasic fragment shader. The fog uses distance from the camera view to change the colour of things further away, thus obsuring them. The alpha discard uses an alpha map to remove parts from a model texture and make them transparent. This was done using the dirt texture from cw1's texture mixing for an alpha map.

## Demo of scene - cw1

[video](https://www.youtube.com/watch?v=KauvRVzMQLs)


## Demo of scene - cw2

[video](https://www.youtube.com/watch?v=p3HnVZ34Vs8)

