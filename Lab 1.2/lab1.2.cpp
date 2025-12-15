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

// Assignment: Make a substantially different pattern, brick-like pattern or other patterns.

// Making a brick like pattern with shifted rows (+radially!)

float fract(float a){
    return a - floor(a);
}

void maketexture()
{
    int x, y;
    float brickHeight = 40;
    float brickWidth = 100;

    for(x = 0; x < kTextureSize; x++)
    for(y = 0; y < kTextureSize; y++)
    {
        // Offset every other row
        // (int)(x / brickHeight) - Row number (floored with int)
        // Every odd row multiplies with half a brick width
        float rowOffset = ((int)(x / brickHeight) % 2) * (brickWidth / 2.0);

        // Dividing x with brickHeight and y (+offset) with brickWidth gives us a
        // normalized "brick space".
        // fx AND fy is for sure within "brick space" now
        float fx = fract((x) / brickHeight);
        float fy = fract((y + rowOffset) / brickWidth);

        // We coloring within brick space, leaving a 0.1 margin on either side for mortar
        if ((fx > 0.1) && (fx < 0.9) &&
            (fy > 0.1) && (fy < 0.9))
        {
            // Color: brick orange
            float noise = fract(sin(x * 12.9898 + y * 78.233) * 12345.6789);
            ptex[x][y][0] = 200 + noise * 20;   // Vary between 200 - 220
            ptex[x][y][1] = 102 + noise * 10;   // Vary between 102 - 112
            ptex[x][y][2] = 31 + noise * 5;     // Vary between 31 - 36
        }
        else
        {
            // Color: mortar gray (default)
            float noise = fract(sin(x * 12.9898 + y * 78.233) * 12345.6789);
            ptex[x][y][0] = 128 + noise * 10;
            ptex[x][y][1] = 128 + noise * 10;
            ptex[x][y][2] = 128 + noise * 10;
        }
    }
}

void maketexture_radial()
{
    int x,y;
    int centerX = kTextureSize / 2;
    int centerY = kTextureSize / 2;

    float brickHeight = 20;   // radial thickness
    float brickArc = 0.2;     // angular width in radians

    for (x = 0; x < kTextureSize; x++)
    for (y = 0; y < kTextureSize; y++)
    {
        // Convert to polar coordinates
        float dx = x - centerX;             // Position
        float dy = y - centerY;             // Position
        float r = sqrt(dx * dx + dy * dy);  // Distance from center pole
        float theta = atan2(dy, dx);        // Angle (positive --> anticlockwise)

        // Normalize theta to [0, 2pi]
        if (theta < 0) theta += 2.0 * M_PI;

        // Offset every other ring
        float offset = ((int)(r / brickHeight) % 2) * (brickArc/ 2.0);
        float fx = fract((r) / brickHeight);
        float fy = fract((theta + offset) / brickArc);

        // Brick texture
        if((fx > 0.1) && (fx < 0.9) &&
           (fy > 0.1) && (fy < 0.9))
        {
            float noise = fract(sin(x * 12.9898 + y * 78.233) * 12345.6789);
            ptex[x][y][0] = 200 + noise * 20;   // Vary between 200 - 220
            ptex[x][y][1] = 102 + noise * 10;   // Vary between 102 - 112
            ptex[x][y][2] = 31 + noise * 5;     // Vary between 31 - 36
        }
        else
        {
            // Color: mortar gray (default)
            float noise = fract(sin(x * 12.9898 + y * 78.233) * 12345.6789);
            ptex[x][y][0] = 128 + noise * 10;
            ptex[x][y][1] = 128 + noise * 10;
            ptex[x][y][2] = 128 + noise * 10;
        }

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

//	maketexture();
	maketexture_radial();

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
	init ();
	glutMainLoop();
}
