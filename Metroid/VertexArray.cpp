#include"VertexArray.h"

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &vertexArray);
}

// Links a VBO to the VAO using a certain layout
void VertexArray::LinkVertexBuffer(VertexBuffer& vertexBuffer, GLuint layout)
{
	//Configure & enable vertex array attribute
	vertexBuffer.bind();
	glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(layout);
	vertexBuffer.unbind();
}

// Binds the VAO
void VertexArray::bind()
{
	glBindVertexArray(vertexArray);
}

// Unbinds the VAO
void VertexArray::unbind()
{
	glBindVertexArray(0);
}

// Deletes the VAO
void VertexArray::destroy()
{
	glDeleteVertexArrays(1, &vertexArray);
}