#pragma once

#include <string>
#include "glad/glad.h"
#include "Core/Ref.h"

class Shader : public RefCounted
{
public:
	Shader(const std::string& vertexSrc, const std::string& fragmentSrc);

	void Bind() const;

	GLuint GetRendererID() const { return m_RendererID; }
private:
	GLuint RT_Compile(const std::string& vertexSrc, const std::string& fragmentSrc);
private:
	GLuint m_RendererID = 0;
};
