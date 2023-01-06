#ifndef VERTEX_ARRAY_CLASS_H
#define VERTEX_ARRAY_CLASS_H

#include<glad/glad.h>
#include"VertexBuffer.h"

class VertexArray
{
public:
	GLuint vertexArray;

	VertexArray();

	void LinkVertexBuffer(VertexBuffer& vexterBuffer, GLuint layout);


	void bind();

	void unbind();

	void destroy();
};

#endif