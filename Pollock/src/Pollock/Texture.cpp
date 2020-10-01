#include "Texture.h"

#include <glad/glad.h>

#include "../vendor/stb_image/stb_image.h"

Texture2D::Texture2D(const std::string& path)
	: m_Path(path)
{
	int width, height, bpp;
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb_alpha);

	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture2D::Bind(uint32_t slot)
{
	glBindTextureUnit(slot, m_RendererID);
}

void Texture2D::Unbind()
{
	glBindTextureUnit(0, m_RendererID);
}
