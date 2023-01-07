#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Include GLFW
#include <GLFW/glfw3.h>
//#include<glad/glad.h>

// Assimp includes
#include <assimp/cimport.h> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

// Project includes
#include "maths_funcs.h"

#define MESH_NAME "spider.dae"
#define INTERVAL 15

typedef struct a
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;

GLFWwindow* window;

using namespace std;
GLuint shaderProgramID;
GLuint vertexbuffer;
GLuint colorbuffer;
GLuint normalbuffer;
GLuint texturebuffer;
GLuint loc1, loc2, loc3, loc4;
GLfloat rotate_y = 0.0f;

ModelData mesh_data;
ModelData dragon_data;

unsigned int mesh_vao = 0;
int width = 0;
int height = 0;
float deltaAngle = 0.0f;
float deltaMove = 0;
// angle of rotation for the camera direction
float angle = 0.0;
// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;
// XZ position of the camera
float x = 0.0f, z = 5.0f;
GLboolean zooming;
GLfloat fov;
GLfloat aspect = 1.0f;
void zoom_in();
void zoom_out();
GLfloat prev_mouse_X;
GLfloat prev_mouse_Y;
bool is_first_time = true;
vec2 m_mousePos;

// position
vec3 fposition = vec3(0, 0, 5);
// horizontal angle : toward -Z
float fhorizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float fverticalAngle = 0.0f;
// Initial Field of View
float finitialFoV = 45.0f;

float fspeed = 3.0f; // 3 units / second
float fmouseSpeed = 0.005f;

mat4 ViewMatrix = identity_mat4();
mat4 ProjectionMatrix = perspective(fov, aspect, 0.1f, 1000.0f);

bool is_updated = false;

// Name of shader files
const char* pVSFileName = "simpleVertexShader.txt";
const char* pFSFileName = "simpleFragmentShader.txt";

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// Handle for shader
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const char* pShaderSource = readShaderSource(pShaderText);

	// To load shader text onto shader handle
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	// Allocate handle for our program
	shaderProgramID = glCreateProgram();
	// Check for non-zero handle as system may sometimes run out of resources
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, pVSFileName, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, pFSFileName, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

void generateObjectBufferMesh() {

	mesh_data = load_mesh(MESH_NAME);

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	loc4 = glGetAttribLocation(shaderProgramID, "vertex_color");

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);


	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	/*glGenBuffers (1, &texturebuffer);
	glBindBuffer (GL_ARRAY_BUFFER, texturebuffer);
	glBufferData (GL_ARRAY_BUFFER, mesh_data.mTextureCoords[0] * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);*/

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	glEnableVertexAttribArray (loc3);
	//	glBindBuffer (GL_ARRAY_BUFFER, texturebuffer);
	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);


	glEnableVertexAttribArray(loc4);
	glVertexAttribPointer(loc4, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void change_viewport(int w, int h) {
	// Set the viewport to be the entire window
	aspect = GLfloat(width) / height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 
}

void RenderScreen() {

	// Dark background
	glClearColor(0.f, 0.0f, 0.4f, 1.0f);

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(shaderProgramID);

	if (zooming) zoom_in();
	else if (fov <= 90) zoom_out();

	// Get a handle for our "LightPosition" uniform
	GLuint LightID = glGetUniformLocation(shaderProgramID, "LightPosition_worldspace");


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "projection");
	int MVP_location = glGetUniformLocation(shaderProgramID, "MVP");

	// Compute the MVP matrix from keyboard and mouse input
	mat4 View = ViewMatrix;
	mat4 Projection = ProjectionMatrix;
	mat4 Model = identity_mat4();
	//Model = rotate_z_deg(Model, rotate_y);
	//View = translate(View, vec3(0.0, 0.0, -10.0f));
	mat4 MVP = Projection * View * Model;

	Model = rotate_y_deg(View, 180.0f);
	View = translate(View, vec3(0.0, 0.0, -20.0f));

	glPushMatrix();
	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, Projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, View.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Model.m);
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, MVP.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	glPopMatrix();

	glPushMatrix();
	// set up the child matrix
	mat4 modelchild_1 = identity_mat4();
	modelchild_1 = rotate_z_deg(modelchild_1, 0.0f);
	modelchild_1 = scale(modelchild_1, vec3(1/2.0f, 1/2.0f, 1/2.0f));
	modelchild_1 = translate(modelchild_1, vec3(-30.0f, rotate_y, 20.0f));

	// Apply the root matrix to the child matrix
	modelchild_1 = Model * modelchild_1;

	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelchild_1.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	glPopMatrix();

	
	glPushMatrix();
	// set up the second child matrix
	mat4 modelchild_2 = identity_mat4();
	modelchild_2 = scale(modelchild_2, vec3( 1.0f,  1.0f,  1.0f));
	modelchild_2 = translate(modelchild_2, vec3(0.0f, 0.0f, 60.0f));

	// Apply the root matrix to the child matrix
	modelchild_2 = Model * modelchild_2;

	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelchild_2.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	glPopMatrix();

	vec3 lightPos = vec3(4, 4, 4);
	//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	glutSwapBuffers();
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly up and down around the y axis
	if (rotate_y >= 0) {
		rotate_y += 8.0f * delta;
		rotate_y = fmodf(rotate_y, 360.0f);
		//printf("%f\n", rotate_y);

		//Reset if it reaches the end of window
		if (rotate_y >= 20.0f) {
			rotate_y = 0.0f;
			//printf("decreasing\n");
			//printf("%f\n", rotate_y);
		}
	}

	// Draw the next frame
	glutPostRedisplay();
}

void timer(int value) {
	if (is_updated) {
		is_updated = false;
		glutPostRedisplay();
	}
	glutTimerFunc(INTERVAL, timer, 0);
}

void init() {
	// Set up the shaders
	shaderProgramID = CompileShaders();

	// load mesh into a vertex buffer array
	generateObjectBufferMesh();

}

void pressKey(int key, int xx, int yy) {
	float fraction = 0.01f;


	switch (key) {
	case GLUT_KEY_LEFT:
		angle -= 0.01f;
		lx = sin(angle);
		lz = -cos(angle);
		deltaAngle = -0.01f; 
		break;

	case GLUT_KEY_RIGHT:
		angle += 0.01f;
		lx = sin(angle);
		lz = -cos(angle);
		deltaAngle = 0.01f; 
		break;

	case GLUT_KEY_UP:
		x += lx * fraction;
		z += z * fraction;
		deltaMove = -0.5f;
		break;

	case GLUT_KEY_DOWN:
		x -= x * fraction;
		z -= z * fraction;
		deltaMove = 0.5f;
		break;
	}

	vec3 position = vec3(x, 1.0f, z);
	vec3 up = vec3(0.0f, -1.0f, 0.0f);
	vec3 direction = vec3(x + lx, 1.0f, z + lz);

	ProjectionMatrix = perspective(45.0f, aspect, 0.1f, 1000.0f);

	ViewMatrix = look_at(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
	glutPostRedisplay();
}

void callback_mouse_button(int button, int state, int x, int y) {
	printf("%d, %d\n", button, state);
	
	if (button == GLUT_LEFT_BUTTON) {
		if (state == 0) {
			zooming = true;
		}
		else {
			zooming = false;
		}
	}
}

void zoom_in() {
	// FOV == field of view of the camera
	// zooming is set by the callback_mouse_button()
	fov -= 10.0f;
	if (fov < 5.0f) fov = 5.0f; // don't allow fov less than 5
	ProjectionMatrix = perspective(fov, aspect, 1.0f, 1000.0f);
}

void zoom_out() {
	// FOV == field of view of the camera
	// zooming is set by the callback_mouse_button()
	fov += 10.0f;
	if (fov > 60.0f) fov = 60.0f; // don't allow fov less than 5
	ProjectionMatrix = perspective(fov, aspect, 1.0f, 1000.0f);
}

// Compare the current mouse position with the old one to figure out which way to rotate the camera.
// We call these "deltas" or "differences" in x and y
void look(int x, int y) {
	if (is_first_time) {
		if (zooming)
			is_first_time = false;
		prev_mouse_X = GLfloat(x);
		prev_mouse_Y = GLfloat(y);
		return;
	}

	GLfloat deltaX = prev_mouse_X - x;
	GLfloat deltaY = prev_mouse_Y - y;

	if (deltaX < -6.0f) { deltaX = -1.0f; }
	if (deltaX > 6.0f) { deltaX = 1.0f; }
	if (deltaY < -6.0f) { deltaY = -1.0f; }
	if (deltaY > 6.0f) { deltaY = 1.0f; }

	if (zooming) {
		deltaX /= 10.0f;
		deltaY /= 10.0f;
	}

	ViewMatrix = rotate_x_deg(identity_mat4(), (-deltaY) / (50 * 5));
	ViewMatrix = rotate_y_deg(ViewMatrix, (-deltaX) / (20 * 5));
	ViewMatrix = rotate_z_deg(ViewMatrix, (-deltaX) / (70 * 5));

	/*else {
		camera->rotateX(deltaY / 150.0f);
		camera->rotateY(deltaX / 150.0f);
	}*/

	// 7) Update prev_mouse_X and prev_mouse_Y to be the current x and y
	prev_mouse_X = x;
	prev_mouse_Y = y;
}

int main(int argc, char** argv) {

	//1.GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	width = glutGet(GLUT_SCREEN_WIDTH);
	height = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowSize(width, height);
	int win = glutCreateWindow("Metroid");
	printf("Window ID: %d\n", win);

	//2. GLEW initialization - must be done after GLUT is initialized
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	//3. Clear screen to remove junk
	GLclampf R = 0.0f, G = 0.0f, B = 0.0f, A = 0.0f;
	glClearColor(R, G, B, A);

	//4. Display functions
	glutReshapeFunc(change_viewport);
	glutDisplayFunc(RenderScreen);
	glutIdleFunc(updateScene);

	// Keyboard movements
	glutSpecialFunc(pressKey);

	// Mouse movements
	glutMouseFunc(callback_mouse_button);
	glutPassiveMotionFunc(look);
	glutMotionFunc(look);

	//5. Set up objects and shaders
	init();

	//6. Begin infinite event loop
	glutMainLoop();

	return 0;
}
