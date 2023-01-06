#include"Shader.h"

char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) {
		return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);
	return buf;
}

Shader::Shader(const char* vertexFileName, const char* fragmentFileName)
{
	//1. Create vertex shader
	vertexShaderID = createShader(GL_VERTEX_SHADER, vertexFileName);

	//2. Create fragment shader
	fragmentShaderID = createShader(GL_FRAGMENT_SHADER, fragmentFileName);

	//3. Create shader program
	shaderProgramID = glCreateProgram();

	// Error check for zero ID
	checkCreationErrors(shaderProgramID, "PROGRAM");
	
	//4. Attach vertex and fragment shader to shader program
	glAttachShader(shaderProgramID, vertexShaderID);
	glAttachShader(shaderProgramID, fragmentShaderID);

	//5. Link shader program
	glLinkProgram(shaderProgramID);

	// Error check - program errors
	checkCompileErrors(shaderProgramID, "PROGRAM");

	// Error check - program validation
	glValidateProgram(shaderProgramID);
	checkCompileErrors(shaderProgramID, "PROGRAM");

	useProgram();
}

GLuint Shader::createShader(GLenum shaderType, const char* shaderFileName) {
	// Create handle for shader
	GLuint shaderID = glCreateShader(shaderType);

	// Error check
	checkCreationErrors(shaderID, "V/F");

	// Read source from file
	const char* shaderSource = readShaderSource(shaderFileName);

	// Attach source to vertex shader object
	glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

	// Compile shader
	glCompileShader(shaderID);

	// Error check - either vertext or fragment shader type
	checkCompileErrors(shaderID, "V/F");
	
	return shaderID;
}

void Shader::useProgram()
{
	glUseProgram(shaderProgramID);
}

void Shader::deleteProgram()
{
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glDeleteProgram(shaderProgramID);
}

void Shader::checkCreationErrors(unsigned int shaderID, const char* type) {
	if (shaderID == 0) {
		fprintf(stderr, "Error creating shader type %s\n", type);
		exit(0);
	}
}

void Shader::checkCompileErrors(unsigned int shaderID, const char* type) {
	GLint success;
	GLchar InfoLog[1024] = { '\0' };

	if (type == "PROGRAM") {
		glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);

		if (!success) {
			glGetProgramInfoLog(shaderProgramID, sizeof(InfoLog), NULL, InfoLog);
			fprintf(stderr, "Invalid shader program: '%s'\n", InfoLog);
			exit(1);
		}
	}
	else {
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

		if (!success) {
			glGetShaderInfoLog(shaderID, 1024, NULL, InfoLog);
			fprintf(stderr, "Error compiling shader: '%s'\n", InfoLog);
			exit(1);
		}	
	}
}