#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include"Shader.h"
#include"VertexBuffer.h"
#include"VertexArray.h"

// traingle example - delete later
// Vertices coordinates
GLfloat vertices[] =
{
	-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower left corner
	0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower right corner
	0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f // Upper corner
};

// Variable declarations
using namespace std;

GLFWwindow* window;

GLuint shaderProgramID;
GLuint vertexShaderID;
GLuint fragmentShaderID;

GLuint vertexbuffer;
GLuint vertexArray;
GLuint colorbuffer;
GLuint normalbuffer;
GLuint texturebuffer;

// Name of shader files
const char* pVSFileName = "vertex.txt";
const char* pFSFileName = "fragment.txt";

Shader shader(pVSFileName, pFSFileName);
VertexArray vertexArray1;
VertexBuffer vertexBuffer1(vertices);

//char* readShaderSource(const char* shaderFile) {
//	FILE* fp;
//	fopen_s(&fp, shaderFile, "rb");
//
//	if (fp == NULL) {
//		return NULL; 
//	}
//
//	fseek(fp, 0L, SEEK_END);
//	long size = ftell(fp);
//	fseek(fp, 0L, SEEK_SET);
//	char* buf = new char[size + 1];
//	fread(buf, 1, size, fp);
//	buf[size] = '\0';
//
//	fclose(fp);
//	return buf;
//}

//GLuint createShader(GLenum shaderType, const char* shaderFileName) {
//	// Create handle for shader
//	GLuint shaderID = glCreateShader(shaderType);
//
//	// Error check
//	if (shaderID == 0) {
//		fprintf(stderr, "Error creating shader type %d\n", shaderType);
//		exit(0);
//	}
//
//	// Read source from file
//	const char* shaderSource = readShaderSource(shaderFileName);
//
//	// Attach source to vertex shader object
//	glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);
//
//	// Compile shader
//	glCompileShader(shaderID);
//
//	// Error check
//	GLint success;
//	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
//
//	if (!success) {
//		GLchar InfoLog[1024] = { '\0' };
//		glGetShaderInfoLog(shaderID, 1024, NULL, InfoLog);
//		fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, InfoLog);
//		exit(1);
//	}
//
//	return shaderID;
//}

//GLuint compileShaders() {
//	//1. Create vertex shader
//	vertexShaderID = createShader(GL_VERTEX_SHADER, pVSFileName);
//
//	//2. Create fragment shader
//	fragmentShaderID = createShader(GL_FRAGMENT_SHADER, pFSFileName);
//
//	//3. Create shader program
//	shaderProgramID = glCreateProgram();
//
//	// Error check for zero ID
//	if (shaderProgramID == 0) {
//		fprintf(stderr, "Error creating shader program\n");
//		exit(1);
//	}
//
//	//4. Attach vertex and fragment shader to shader program
//	glAttachShader(shaderProgramID, vertexShaderID);
//	glAttachShader(shaderProgramID, fragmentShaderID);
//
//	//5. Link shader program
//	glLinkProgram(shaderProgramID);
//
//	// Error check - program errors
//	GLint Success = 0;
//	GLchar ErrorLog[1024] = { '\0' };
//	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
//	if (Success == 0) {
//		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
//		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
//		exit(1);
//	}
//
//	// Error check - program validation
//	glValidateProgram(shaderProgramID);
//	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
//	if (!Success) {
//		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
//		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
//		exit(1);
//	}
//
//	//6. Use shader program
//	glUseProgram(shaderProgramID);
//
//	return shaderProgramID;
//}

void generateBuffers() {
//	// Generate & bind - vertex buffer object
//	glGenBuffers(1, &vertexbuffer);
//	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	// Generate & bind - vertex array object
//	glGenVertexArrays(1, &vertexArray);
//	glBindVertexArray(vertexArray);
//	
//	
//	//Configure & enable vertex array attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);

	vertexArray1.bind();
	vertexArray1.LinkVertexBuffer(vertexBuffer1, 0);

	vertexArray1.unbind();
	vertexBuffer1.unbind();
}

void init() {
	//1. Set up shaders
	shaderProgramID = shader.shaderProgramID;

	//2. Set up buffers
	generateBuffers();
}

void destroy(GLFWwindow* window) {
	// Delet all the objects created

	vertexArray1.destroy();
	//glDeleteVertexArrays(1, &vertexArray);

	vertexBuffer1.destroy();
	//glDeleteBuffers(1, &vertexbuffer);

	shader.deleteProgram();
	//glDeleteProgram(shaderProgramID);

	glfwDestroyWindow(window);
	glfwTerminate();
}

int main()
{
	// 1. GLFW Initialization
	glfwInit();

	// Mention OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	// Create window
	window = glfwCreateWindow(900, 900, "Metroid", NULL, NULL);

	// Error check - for null window
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);

	//2. GLAD loading
	gladLoadGL();
	glViewport(0, 0, 800, 800);

	//3. Clear screen to remove junk
	GLclampf R = 0.0f, G = 0.0f, B = 0.0f, A = 0.0f;
	glClearColor(R, G, B, A);

	// 4. Add Background color
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	//5. Set up objects and shaders
	init();

	//6. Begin infinite event loop
	while (!glfwWindowShouldClose(window))
	{
		// Blue background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		shader.useProgram();
		//glUseProgram(shaderProgramID);
		// Bind the VAO so OpenGL knows to use it
		glBindVertexArray(vertexArray);
		// Draw the triangle using the GL_TRIANGLES primitive
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	//7. Destroy
	destroy(window);
	return 0;
}