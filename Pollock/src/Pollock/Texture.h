#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include "Base.h"

enum class TextureFilter
{
	None = 0, Linear, Nearest
};

enum class TextureWrap
{
	None = 0, Clamp, Repeat
};

struct TextureProperties
{
	TextureFilter Filter = TextureFilter::Linear;
	TextureWrap Wrap = TextureWrap::Clamp;
};

class Image2D : public RefCounted
{
public:
	Image2D(uint32_t width, uint32_t height); // TODO: implement properly
	Image2D(const std::string& path);
	~Image2D();

	void Release();

	template<typename T>
	T* GetData()
	{
		return (T*)m_Data;
	}

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	const std::string& GetPath() const { return m_Path; }
private:
	unsigned char* m_Data = nullptr;
	uint32_t m_Width = 0, m_Height = 0;

	std::string m_Path;

	friend class Texture2D;
};

class Texture2D : public RefCounted
{
public:
	Texture2D(uint32_t width, uint32_t height, const void* data); // Format is RGBA
	Texture2D(const std::string& path, const TextureProperties& textureProperties = TextureProperties());
	Texture2D(Ref<Image2D> image, const TextureProperties& textureProperties = TextureProperties());
	~Texture2D();

	void Bind(uint32_t slot = 0);
	void Unbind();

	void SetFilter(TextureFilter filter);
	TextureFilter GetFilter() const { return m_Filter; }

	uint32_t GetRendererID() const { return m_RendererID; }

	uint32_t GetWidth() const { return m_Image->GetWidth(); }
	uint32_t GetHeight() const { return m_Image->GetHeight(); }

	const std::string& GetPath() const { return m_Path; }
private:
	void Invalidate();
	void RT_Invalidate();
private:
	uint32_t m_RendererID = 0;
	std::string m_Path;
	Ref<Image2D> m_Image;
	TextureProperties m_Properties;

	uint8_t* m_LocalStorage = nullptr;

	TextureFilter m_Filter;
};

class SubTexture2D : public RefCounted
{
public:
	SubTexture2D(const Ref<Texture2D>& texture, int horizontalSpriteCount, int verticalSpriteCount, int framerate = 15);

	const glm::vec2* GetTextureCoords(int x, int y) const { return &m_TextureCoords[(x + y * m_HorizontalSpriteCount) * 4]; }
	Ref<Texture2D> GetTexture() const { return m_Texture; }
	int GetHorizontalSpriteCount() const { return m_HorizontalSpriteCount; }
	int GetVerticalSpriteCount() const { return m_VerticalSpriteCount; }

	int GetFrameRate() const { return m_FrameRate; }
	void SetFrameRate(int framerate) { m_FrameRate = framerate; }
private:
	Ref<Texture2D> m_Texture;
	std::vector<glm::vec2> m_TextureCoords;
	int m_HorizontalSpriteCount, m_VerticalSpriteCount;
	int m_FrameRate;
};