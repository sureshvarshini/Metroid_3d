#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
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
#include<glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include GLFW
#include <GLFW/glfw3.h>

// Assimp includes
#include <assimp/cimport.h> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

// Project includes
#include "maths_funcs.h"

#define MESH_NAME "spider.dae"
#define INTERVAL 15

//  Sun coordinated
const char* sun_path = "sun.obj";
std::vector<vec3> sun_vertices;
vec3 sun_position = vec3(15.0, 15.0, 15.0);
GLuint sunPosID;
vec3 sunPosition;
GLuint sunDistance;
GLuint sun_VertexArrayID;
GLuint sun_vertexbuffer_size;
GLuint sun_vertexbuffer;
GLuint ModelSunID;
GLuint sunShaderID;

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
float yaw = -90.0f;
float pitch = 0.0f;

glm::vec3 position = glm::vec3(0.0f, 0.5f, 3.0f);
glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
GLboolean zooming;
GLfloat fov;
GLfloat aspect = 1.0f;
void zoom_in();
void zoom_out();
GLfloat prev_mouse_X;
GLfloat prev_mouse_Y;
bool is_first_time = true;
vec2 m_mousePos;

float fspeed = 3.0f;
float fmouseSpeed = 0.005f;

glm::mat4 ViewMatrix = glm::mat4();
glm::mat4 ProjectionMatrix = glm::perspective(fov, aspect, 0.1f, 1000.0f);

bool is_updated = false;

// Name of shader files
const char* pVSFileName = "simpleVertexShader.txt";
const char* pFSFileName = "simpleFragmentShader.txt";

const char* sunVertexShader = "sunVertexShader.txt";
const char* sunFragmentShader = "sunFragmentShader.txt";

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
			if (mesh->HasTangentsAndBitangents()) { }
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

// load obj file
bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices) {
	printf("Loading OBJ file %s...\n", path);
	std::vector<unsigned int> vertexIndices;
	std::vector<glm::vec3> temp_vertices;
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file\n");
		return false;
	}
	while (1) {
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
			if (matches != 6) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
		}
		else {
			char tempBuffer[1000];
			fgets(tempBuffer, 1000, file);
		}
	}
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		out_vertices.push_back(vertex);
	}
	fclose(file);
	return true;
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
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(shaderProgramID, pVSFileName, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, pFSFileName, GL_FRAGMENT_SHADER);

	AddShader(sunShaderID, sunVertexShader, GL_VERTEX_SHADER);
	AddShader(sunShaderID, sunFragmentShader, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	glLinkProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
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

	//if (!loadOBJ(sun_path, sun_vertices)) throw "Error importing OBJ file for the sun";
	//for (unsigned int i = 0; i < sun_vertices.size(); i++) {
	//  sun_vertices.at(i) += sun_position;
	//  sun_vertices.at(i) *= 3; //Scale
	//}
	//sunDistance = (GLuint)sunPosition.y;
	//sunDistance = (GLuint)sunPosition.y;
	//// Generate vertices for sun
	//glGenVertexArrays(1, &sun_VertexArrayID);
	//glBindVertexArray(sun_VertexArrayID);
	//std::vector<GLfloat> result;
	//for (unsigned int i = 0; i < sun_vertices.size(); i++) {
	//  result.push_back(sun_vertices.at(i).x);
	//  result.push_back(sun_vertices.at(i).y);
	//  result.push_back(sun_vertices.at(i).z);
	//}
	//std::vector<GLfloat> g_sun_vertex_buffer_data = result;  
	//sun_vertexbuffer_size = g_sun_vertex_buffer_data.size() / 3;
	//glGenBuffers(1, &sun_vertexbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, sun_vertexbuffer);
	//glBufferData(GL_ARRAY_BUFFER, g_sun_vertex_buffer_data.size() * sizeof(GLfloat), &g_sun_vertex_buffer_data.front(), GL_STATIC_DRAW);
	//// Get a handle for the sun's model transformation
	//ModelSunID = glGetUniformLocation(sunShaderID, "sunPosition_modelspace");
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
	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(shaderProgramID);

	if (zooming) zoom_in();
	else if (fov <= 90) zoom_out();

	// Get a handle for our "LightPosition" uniform
	GLuint LightID = glGetUniformLocation(shaderProgramID, "LightPosition_worldspace");

	//Get a handle for the camera position
	GLuint cameraPosID = glGetUniformLocation(shaderProgramID, "cameraPos");
	// Get a handle for the center of the screen's center coordinates
	GLuint scrID = glGetUniformLocation(shaderProgramID, "scr_center");
	vec2 scr_center = vec2(width / 2, height / 2);
	sunPosID = glGetUniformLocation(shaderProgramID, "lightPos");

	GLfloat cameraPosition[] = { position.x, position.y, position.z };
	glUniform3fv(cameraPosID, 1, &cameraPosition[0]);
	glUniform2fv(scrID, 1, &scr_center.v[0]);

	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "projection");
	int MVP_location = glGetUniformLocation(shaderProgramID, "MVP");

	glm::mat4 View = ViewMatrix;
	glm::mat4 Projection = ProjectionMatrix;
	glm::mat4 Model;
	glm::mat4 MVP = Projection * View * Model;

	Model = glm::rotate(View, 180.0f, glm::vec3(0, 1, 0));
	View = glm::translate(View, glm::vec3(0.0, 0.0, -20.0f));

	glPushMatrix();
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(View));
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));

	// Front spider
	glm::mat4 front_spider;
	front_spider = Model * front_spider;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(front_spider));
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	glPopMatrix();

	//Up-down-spider - right
	glPushMatrix();
	glm::mat4 right_spider;
	right_spider = glm::rotate(right_spider, 0.0f, glm::vec3(0, 0, 1));
	right_spider = glm::scale(right_spider, glm::vec3(0.2f, 0.2f, 0.2f));
	right_spider = glm::translate(right_spider, glm::vec3(-30.0f, rotate_y, 20.0f));

	right_spider = Model * right_spider;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(right_spider));
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	glPopMatrix();

	//Back left side spider
	glPushMatrix();
	glm::mat4 back_spider;
	back_spider = glm::scale(back_spider, glm::vec3(0.2f, 0.2f, 0.2f));
	back_spider = glm::translate(back_spider, glm::vec3(50.0f, 0.0f, 60.0f));

	back_spider = Model * back_spider;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(back_spider));
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
	glPopMatrix();

	glm::vec3 lightPos = glm::vec3(4, 4, 4);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

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

		//Reset if it reaches the end of window
		if (rotate_y >= 20.0f) {
			rotate_y = 0.0f;
		}
	}

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
	float fraction = 0.5f;

	switch (key) {
	case GLUT_KEY_LEFT:
		position -= glm::normalize(glm::cross(direction, up)) * fraction;
		break;

	case GLUT_KEY_RIGHT:
		position += glm::normalize(glm::cross(direction, up)) * fraction;
		break;

	case GLUT_KEY_UP:
		position += fraction * direction;
		break;

	case GLUT_KEY_DOWN:
		position -= fraction * direction;
		break;
	}


	ProjectionMatrix = glm::perspective(45.0f, aspect, 0.1f, 1000.0f);

	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up
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
	fov -= 10.0f;
	if (fov < 5.0f) fov = 5.0f;
	ProjectionMatrix = glm::perspective(fov, aspect, 1.0f, 1000.0f);
}

void zoom_out() {
	fov += 10.0f;
	if (fov > 60.0f) fov = 60.0f;
	ProjectionMatrix = glm::perspective(fov, aspect, 1.0f, 1000.0f);
}

void look(int x, int y) {
	float sens = 0.1f;
	if (is_first_time) {
		is_first_time = false;
		prev_mouse_X = GLfloat(x);
		prev_mouse_Y = GLfloat(y);
		return;
	}

	GLfloat deltaX = x - prev_mouse_X;
	GLfloat deltaY = prev_mouse_Y - y;

	prev_mouse_X = x;
	prev_mouse_Y = y;

	yaw += deltaX;
	pitch += deltaY;

	if (zooming) {
		deltaX /= 10.0f;
		deltaY /= 10.0f;
	}

	glm::vec3 cam;
	cam.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	cam.y = sin(glm::radians(pitch));
	cam.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction = glm::normalize(cam);

	printf("Delta %f %f %f\n", direction[0], direction[1], direction[2]);
	ViewMatrix = glm::lookAt(
		position,
		direction,
		up
	);

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

	//5. Keyboard movements
	glutSpecialFunc(pressKey);

	//6. Mouse movements
	glutMouseFunc(callback_mouse_button);
	glutPassiveMotionFunc(look);
	// glutMotionFunc(look);

	//7. Set up objects and shaders
	init();

	//8. Begin infinite event loop
	glutMainLoop();

	return 0;
}
