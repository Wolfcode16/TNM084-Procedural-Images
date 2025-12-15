// TNM084 Lab 1
// Procedural textures
// Draws concentric circles on CPU and GPU
// Make something more interesting!

// For the CPU part, you should primarily work in maketexture() below.

// New C++ version!

//uses framework Cocoa
//uses framework OpenGL
#define MAIN
#include "MicroGlut.h"
#include "GL_utilities.h"
#include <math.h>
#include <stdio.h>
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"

// Lab 1 specific
// Code for you to access to make interesting patterns.
// This is for CPU. For GPU, I provide separate files
// with functions that you can add to your fragment shaders.
#include "noise1234.h"
#include "simplexnoise1234.h"
#include "cellular.h"

#define kTextureSize 512
GLubyte ptex[kTextureSize][kTextureSize][3];
const float ringDensity = 20.0;

// Assignment: Use noise functions to produce something

// TODO:
//  - Voronoi cells (Offset with perlin noise)

float fract(float a){
    return a - floor(a);
}

void maketexture_Interesting()
{
    const long ORDER = 2;
    double at[3];
    double F[ORDER];
    double delta[ORDER][3];
    unsigned long ID[ORDER];

    for (int x = 0; x < kTextureSize; x++)
    for (int y = 0; y < kTextureSize; y++)
    {
        double nx = (double)x / kTextureSize;
        double ny = (double)y / kTextureSize;

        // Sample Perlin noise to get distortion offsets
        at[0] = nx * 3;  // Number of cells in window
        at[1] = ny * 3;
        at[2] = 0;
        double offsetX = noise2(at[0], at[1]);  // Returns -1 to 1

        at[0] = nx * 3 + 100;  // Offset seed for different pattern
        at[1] = ny * 3 + 100;
        double offsetY = noise2(at[0], at[1]);

        // Apply the distortion to coordinates
        double distortedX = nx + offsetX * 0.3;  // 0.3 -> warp intensity
        double distortedY = ny + offsetY * 0.3;

        // Worley noise with warped coordinates
        at[0] = distortedX * 5;
        at[1] = distortedY * 5;
        Worley(at, ORDER, F, delta, ID);

        // Give RGB values from cell ID
        double r = ((ID[0] * 14324) % 255);
        double g = ((ID[0] * 1235) % 255);
        double b = ((ID[0] * 39874) % 255);

        // Make edges darker
        double edge = F[1] - F[0];
        double edgeThreshold = 0.05;
        if (edge < edgeThreshold)
        {
            r *= 0.2;
            g *= 0.2;
            b *= 0.2;
        }

        ptex[x][y][0] = r;
        ptex[x][y][1] = g;
        ptex[x][y][2] = b;
    }
}


// Globals
// Data would normally be read from files
vec3 vertices[] = { vec3(-1.0f,-1.0f,0.0f),
					vec3(1.0f,-1.0f,0.0f),
					vec3(1.0f,1.0f,0.0f),
					vec3(-1.0f,1.0f,0.0f),
					vec3(1.0f,1.0f,0.0f),
					vec3(1.0f,-1.0f,0.0f) };
vec2 texCoords[] = {vec2(0.0f,0.0f),
					vec2(1.0f,0.0f),
					vec2(1.0f,1.0f),
					vec2(0.0f,1.0f),
					vec2(1.0f,1.0f),
					vec2(1.0f,0.0f) };
GLuint indices[] =
{
	0, 1, 2,
	0, 2, 3
};

// vertex array object
//unsigned int vertexArrayObjID;
// Texture reference
GLuint texid;
// Switch between CPU and shader generation
int displayGPUversion = 0;
// Reference to shader program
GLuint program;
// The quad
Model* quad;

void init(void)
{
	// two vertex buffer objects, used for uploading the
//	unsigned int vertexBufferObjID;
//	unsigned int texBufferObjID;

	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	printError("GL inits");

	// Load and compile shader
	program = loadShaders("lab1.vert", "lab1.frag");
	glUseProgram(program);
	printError("init shader");

	// Upload geometry to the GPU:

	quad = LoadDataToModel(vertices,NULL,texCoords,NULL,indices,4,6);

	// Texture unit
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0

// Constants common to CPU and GPU
	glUniform1i(glGetUniformLocation(program, "displayGPUversion"), 0); // shader generation off
	glUniform1f(glGetUniformLocation(program, "ringDensity"), ringDensity);

	maketexture_Interesting();

// Upload texture
	glGenTextures(1, &texid); // texture id
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kTextureSize, kTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, ptex);

	// End of upload of geometry
	printError("init arrays");
}

// Switch on any key
void key(unsigned char key, int x, int y)
{
	displayGPUversion = !displayGPUversion;
	glUniform1i(glGetUniformLocation(program, "displayGPUversion"), displayGPUversion); // shader generation off
	printf("Changed flag to %d\n", displayGPUversion);
	glutPostRedisplay();
}

void display(void)
{
	printError("pre display");

	// Get time
	float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glUniform1f(glGetUniformLocation(program, "time"), currentTime);

     // Check if uniform location is valid
    GLint timeLoc = glGetUniformLocation(program, "time");
    if (timeLoc == -1) {
        printf("ERROR: 'time' uniform not found in shader!\n");
    } else {
        glUniform1f(timeLoc, currentTime);
        printf("Time set to: %f (location: %d)\n", currentTime, timeLoc); // Debug
    }

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawModel(quad, program, "in_Position", NULL, "in_TexCoord");

	printError("display");

	glutSwapBuffers();
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(kTextureSize, kTextureSize);
	glutCreateWindow ("Lab 1");
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutIdleFunc(glutPostRedisplay);
	init ();
	glutMainLoop();
}
