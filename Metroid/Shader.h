#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h>
#include<string>
#include<iostream>
#include<iostream>

char* readShaderSource(const char* filename);

class Shader
{
public:
	GLuint shaderProgramID;
	GLuint vertexShaderID;
	GLuint fragmentShaderID;
	
	Shader(const char* vertexFileName, const char* fragmentFileName);
	
	void useProgram();

	void deleteProgram();

private:
	void checkCreationErrors(unsigned int shaderID, const char* type);

	void checkCompileErrors(unsigned int shader, const char* type);

	GLuint createShader(GLenum shaderType, const char* shaderFileName);
};

#endif

