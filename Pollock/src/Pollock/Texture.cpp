#include "Texture.h"

#include <glad/glad.h>
#include <iostream>

#include "../vendor/stb_image/stb_image.h"

#include "Timer.h"

#include "Renderer.h"

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
	m_Image = Ref<Image2D>::Create(path);
	Invalidate();
}

Texture2D::Texture2D(Ref<Image2D> image, const TextureProperties& textureProperties)
	: m_Image(image), m_Path(image->m_Path), m_Properties(textureProperties)
{
	Invalidate();
}

Texture2D::Texture2D(uint32_t width, uint32_t height, const void* data)
	: m_Image(Ref<Image2D>::Create(width, height))
{
	m_LocalStorage = new uint8_t[width * height * 4];
	memcpy(m_LocalStorage, data, width * height * 4);
	Invalidate();
}

Texture2D::~Texture2D()
{
	Renderer::Submit([rendererID = m_RendererID]()
	{
		glDeleteTextures(1, &rendererID);
	});
}

void Texture2D::Bind(uint32_t slot)
{
	Ref<Texture2D> instance = this;
	Renderer::Submit([instance, slot]()
	{
		glBindTextureUnit(slot, instance->m_RendererID);
	});
}

void Texture2D::Unbind()
{
	__debugbreak();

	Renderer::Submit([]()
	{
		glBindTextureUnit(0, 0);
	});
}

void Texture2D::SetFilter(TextureFilter filter)
{
	m_Filter = filter;

	GLenum glFilter = PollockFilterToGLFilter(filter);

	glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, glFilter);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, glFilter);
}

void Texture2D::Invalidate()
{
	Ref<Texture2D> instance = this;
	Renderer::Submit([instance]() mutable
	{
		instance->RT_Invalidate();
	});
}

void Texture2D::RT_Invalidate()
{
	if (m_Image->GetData<void>())
	{
		ScopedTimer timer("Uploading texture '" + m_Image->m_Path + "'");
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Image->m_Width, m_Image->m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Image->m_Data);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_Image->Release();
	}
	else
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Image->m_Width, m_Image->m_Height);
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Image->m_Width, m_Image->m_Height, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalStorage);

		delete[] m_LocalStorage;
		m_LocalStorage = nullptr;
	}

	GLenum filter = PollockFilterToGLFilter(m_Properties.Filter);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filter);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);

	GLenum wrap = PollockWrapToGLWrap(m_Properties.Wrap);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, wrap);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);
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

Image2D::Image2D(uint32_t width, uint32_t height)
	: m_Width(width), m_Height(height)
{
}

Image2D::~Image2D()
{
	Release();
}

void Image2D::Release()
{
	stbi_image_free(m_Data);
	m_Data = nullptr;
}
