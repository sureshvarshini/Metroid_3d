#ifndef VERTEX_BUFFER_CLASS_H
#define VERTEX_BUFFER_CLASS_H

#include<glad/glad.h>

class VertexBuffer
{
public:
	GLuint vertexbuffer;

	VertexBuffer(GLfloat* vertices);

	void bind();

	void unbind();

	void destroy();
};

#endif