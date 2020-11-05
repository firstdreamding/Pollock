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

class Texture2D
{
public:
	Texture2D(const std::string& path, const TextureProperties& textureProperties = TextureProperties());
	~Texture2D();

	void Bind(uint32_t slot = 0);
	void Unbind();

	uint32_t GetRendererID() const { return m_RendererID; }

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	const std::string& GetPath() const { return m_Path; }
private:
	uint32_t m_RendererID = 0;
	std::string m_Path;
	uint32_t m_Width, m_Height;
	TextureProperties m_Properties;
};

class SubTexture2D
{
public:
	SubTexture2D(const Ref<Texture2D>& texture, int horizontalSpriteCount, int verticalSpriteCount);

	glm::vec2* GetTextureCoords(int x, int y) { return &m_TextureCoords[(x + y * m_HorizontalSpriteCount) * 4]; }
	Ref<Texture2D> GetTexture() { return m_Texture; }
	int GetHorizontalSpriteCount() const { return m_HorizontalSpriteCount; }
	int GetVerticalSpriteCount() const { return m_VerticalSpriteCount; }
private:
	Ref<Texture2D> m_Texture;
	std::vector<glm::vec2> m_TextureCoords;
	int m_HorizontalSpriteCount, m_VerticalSpriteCount;
};