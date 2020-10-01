#pragma once

#include <string>

class Texture2D
{
public:
	Texture2D(const std::string& path);
	~Texture2D();

	void Bind(uint32_t slot = 0);
	void Unbind();
private:
	uint32_t m_RendererID = 0;
	std::string m_Path;
};
