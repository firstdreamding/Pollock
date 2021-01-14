#include "Texture.h"

#include <glad/glad.h>
#include <iostream>

#include "../vendor/stb_image/stb_image.h"

#include "Timer.h"

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
	: m_Path(path), m_Properties(textureProperties), m_Filter(textureProperties.Filter)
{
	int width, height, bpp;

	stbi_set_flip_vertically_on_load(1);
	stbi_uc* data = nullptr;
	{
		ScopedTimer timer("Loading image file '" + path + "'");
		data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb_alpha);
	}

	if (!data)
	{
		std::cout << "Could not load texture " << path << std::endl;
		return;
	}

	m_Width = width;
	m_Height = height;

	{
		ScopedTimer timer("Uploading texture '" + path + "'");
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
	}

	stbi_image_free(data);
}

Texture2D::Texture2D(Image2D* image, const TextureProperties& textureProperties /*= TextureProperties()*/)
	: m_Path(image->m_Path), m_Properties(textureProperties)
{
	m_Width = image->m_Width;
	m_Height = image->m_Height;

	{
		ScopedTimer timer("Uploading texture '" + image->m_Path + "'");
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->m_Data);

		GLenum filter = PollockFilterToGLFilter(m_Properties.Filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		GLenum wrap = PollockWrapToGLWrap(m_Properties.Wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
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

void Texture2D::SetFilter(TextureFilter filter)
{
	m_Filter = filter;

	GLenum glFilter = PollockFilterToGLFilter(filter);

	glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, glFilter);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, glFilter);
}

SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, int horizontalSpriteCount, int verticalSpriteCount, int framerate)
	: m_Texture(texture), m_HorizontalSpriteCount(horizontalSpriteCount), m_VerticalSpriteCount(verticalSpriteCount), m_FrameRate(framerate)
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

Image2D::Image2D(const std::string& path)
	: m_Path(path)
{
	int width, height, bpp;

	stbi_set_flip_vertically_on_load(1);
	stbi_uc* data = nullptr;
	{
		ScopedTimer timer("Loading image file '" + path + "'");
		data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb_alpha);
	}

	if (!data)
	{
		std::cout << "Could not load texture " << path << std::endl;
		return;
	}

	m_Data = data;

	m_Width = width;
	m_Height = height;
}

Image2D::~Image2D()
{
	stbi_image_free(m_Data);
}
