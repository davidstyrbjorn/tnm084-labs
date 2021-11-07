// TNM084 Lab 1
// Procedural textures
// Draws concentric circles on CPU and GPU
// Make something more interesting!

// For the CPU part, you should primarily ork in maketexture() below.

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	//uses framework Cocoa
#endif
#include "MicroGlut.h"
#include "GL_utilities.h"
#include <Math.h>
#include <stdio.h>

// Lab 1 specific
// Code for you to access to make interesting patterns.
// This is for CPU. For GPU, I provide sepatrate files
// with functions that you can add to your fragment shaders.
#include "noise1234.h"
#include "simplexnoise1234.h"
#include "cellular.h"

#define kTextureSize 512
GLubyte ptex[kTextureSize][kTextureSize][3];
const float ringDensity = 20.0;

// Example: Radial pattern.
void maketexture()
{
	
	// Changed the texture by multiplying by additional trig functions on lines 50 and 51
	// Also shifted the the B value of final pixel value to 200 for a change in color
	
	int x, y;
	float fx, fy, fxo, fyo;
	char val;
	
	for(int y = 0; y < kTextureSize; y++) {	
		for (x = 0; x < kTextureSize; x++)
		{		
			float xf = (float)x / kTextureSize;
			float yf = (float)y / kTextureSize;
			float freq = 7.;
			float value = pnoise2(xf*freq, yf*freq, kTextureSize, kTextureSize) * 10;
			value = value - (int)value;
			
			// The returned pixel vaue is negative -0.5 to 0.5, so we normalize it to 0->244 below
			float noisePixelValue = 144 + value*144;
						
			ptex[x][y][0] = noisePixelValue * (xf > 0.5 && yf > 0.5);
			ptex[x][y][1] = noisePixelValue * (xf < 0.5);
			ptex[x][y][2] = noisePixelValue * (xf > 0.5 && yf < 0.5);
		}
	}
}

// Globals
// Data would normally be read from files
GLfloat vertices[] = { -1.0f,-1.0f,0.0f,
						-1.0f,1.0f,0.0f,
						1.0f,1.0f,0.0f,
						-1.0f,-1.0f,0.0f,
						1.0f,1.0f,0.0f,
						1.0f,-1.0f,0.0f };
GLfloat texCoords[] = { 0.0f,0.0f,
						0.0f,1.0f,
						1.0f,1.0f,
						0.0f,0.0f,
						1.0f,1.0f,
						1.0f,0.0f };

// vertex array object
unsigned int vertexArrayObjID;
// Texture reference
GLuint texid;
// Switch between CPU and shader generation
int displayGPUversion = 0;
// Reference to shader program
GLuint program;
GLint timeUniformLoc;
float t = 0;

void init(void)
{
	// two vertex buffer objects, used for uploading the
	unsigned int vertexBufferObjID;
	unsigned int texBufferObjID;

	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	printError("GL inits");

	// Load and compile shader
	program = loadShaders("lab1.vert", "lab1.frag");
	glUseProgram(program);
	printError("init shader");
	
	timeUniformLoc = glGetUniformLocation(program, "time");
	
	// Upload gemoetry to the GPU:
	
	// Allocate and activate Vertex Array Object
	glGenVertexArrays(1, &vertexArrayObjID);
	glBindVertexArray(vertexArrayObjID);
	// Allocate Vertex Buffer Objects
	glGenBuffers(1, &vertexBufferObjID);
	glGenBuffers(1, &texBufferObjID);
	
	// VBO for vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(glGetAttribLocation(program, "in_Position"));

	// VBO for texture
	glBindBuffer(GL_ARRAY_BUFFER, texBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(program, "in_TexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(program, "in_TexCoord"));
	
	// Texture unit
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0

// Constants common to CPU and GPU
	glUniform1i(glGetUniformLocation(program, "displayGPUversion"), 0); // shader generation off
	glUniform1f(glGetUniformLocation(program, "ringDensity"), ringDensity);

	maketexture();

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

void onIDLE(){
	float t = glutGet(GLUT_ELAPSED_TIME);
	// Update time variable in shader
	//t += 0.1f;
	glUseProgram(program);
	glUniform1f(timeUniformLoc, t);
	
	glutPostRedisplay();
}

void display(void)
{
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vertexArrayObjID);	// Select VAO
	glDrawArrays(GL_TRIANGLES, 0, 6);	// draw object
	
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
	glutIdleFunc (onIDLE);
	init ();
	glutMainLoop();
}
