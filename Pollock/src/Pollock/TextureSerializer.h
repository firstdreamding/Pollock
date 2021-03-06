#pragma once

#include "Texture.h"

class TextureSerializer
{
public:
	TextureSerializer(Ref<Texture2D>& texture, Ref<SubTexture2D>& subTexture);
	TextureSerializer();

	void Serialize(const std::string& filepath);
	void Deserialize(const std::string& filepath);

	Ref<Texture2D> GetTexture();
	Ref<SubTexture2D> GetSubTexture();

private:
	Ref<Texture2D> m_Texture;
	Ref<SubTexture2D> m_SubTexture;

};
