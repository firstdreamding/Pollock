#include "Texture.h"

#include <glad/glad.h>
#include <iostream>

#include "../vendor/stb_image/stb_image.h"

static GLenum PollockFilterToGLFilter(TextureFilter filter)
{
	switch (filter)
	{
		case TextureFilter::Linear:  return GL_LINEAR;
		case TextureFilter::Nearest: return GL_NEAREST;
	}

	// assert
	return 0;
}

static GLenum PollockWrapToGLWrap(TextureWrap wrap)
{
	switch (wrap)
	{
		case TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
		case TextureWrap::Repeat: return GL_REPEAT;
	}

	// assert
	return 0;
}

Texture2D::Texture2D(const std::string& path, const TextureProperties& textureProperties)
	: m_Path(path), m_Properties(textureProperties)
{
	int width, height, bpp;

	stbi_set_flip_vertically_on_load(1);
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb_alpha);

	if (!data)
	{
		std::cout << "Could not load texture " << path << std::endl;
		return;
	}

	m_Width = width;
	m_Height = height;

	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	GLenum filter = PollockFilterToGLFilter(m_Properties.Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	GLenum wrap = PollockWrapToGLWrap(m_Properties.Wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

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

SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, int horizontalSpriteCount, int verticalSpriteCount)
	: m_Texture(texture), m_HorizontalSpriteCount(horizontalSpriteCount), m_VerticalSpriteCount(verticalSpriteCount)
{
	auto width = texture->GetWidth();
	auto height = texture->GetHeight();

	float spriteWidth = width / (float)horizontalSpriteCount;
	float spriteHeight = height / (float)verticalSpriteCount;

	float spriteWidthTC = spriteWidth / width;
	float spriteHeightTC = spriteHeight / height;

	for (int y = 0; y < verticalSpriteCount; y++)
	{
		for (int x = 0; x < horizontalSpriteCount; x++)
		{
			float u0 = spriteWidthTC * x;
			float v0 = spriteHeightTC * y;

			float u1 = spriteWidthTC * (x + 1);
			float v1 = spriteHeightTC * (y + 1);

			std::cout << "Sprite " << x << ", " << y << std::endl;
			std::cout << u0 << ", " << v0 << std::endl;
			std::cout << u1 << ", " << v1 << std::endl << std::endl;

			m_TextureCoords.emplace_back(u0, v0);
			m_TextureCoords.emplace_back(u1, v0);
			m_TextureCoords.emplace_back(u1, v1);
			m_TextureCoords.emplace_back(u0, v1);
		}
	}

}
