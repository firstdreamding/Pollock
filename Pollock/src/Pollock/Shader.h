#pragma once

#include <string>
#include "glad/glad.h"

class Shader
{
public:
	Shader(const std::string& vertexSrc, const std::string& fragmentSrc);

	void Bind();

	GLuint GetRendererID() const { return m_RendererID; }
private:
	GLuint Compile(const std::string& vertexSrc, const std::string& fragmentSrc);
private:
	GLuint m_RendererID;
};
