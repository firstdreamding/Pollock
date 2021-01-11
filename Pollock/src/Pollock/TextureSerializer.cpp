#include "TextureSerializer.h"

#include "yaml-cpp/yaml.h"

#include <fstream>
#include <iostream>

TextureSerializer::TextureSerializer(Ref<Texture2D>& texture, Ref<SubTexture2D>& subTexture)
	: m_Texture(texture), m_SubTexture(subTexture)
{
}

void TextureSerializer::Serialize(const std::string& filepath)
{
	YAML::Emitter out;
	out << YAML::Comment("Texture");

	out << YAML::BeginMap; // Texture
	out << YAML::Key << "AssetPath" << YAML::Value << m_Texture->GetPath();
	out << YAML::Key << "Filter" << YAML::Value << (int)m_Texture->GetFilter();

	if (m_SubTexture)
	{
		out << YAML::Key << "SubTexture" << YAML::Value;
		out << YAML::BeginMap; // SubTexture
		out << YAML::Key << "FrameRate" << YAML::Value << m_SubTexture->GetFrameRate();
		out << YAML::Key << "HorizontalSpriteCount" << YAML::Value << m_SubTexture->GetHorizontalSpriteCount();
		out << YAML::Key << "VerticalSpriteCount" << YAML::Value << m_SubTexture->GetVerticalSpriteCount();
		out << YAML::EndMap; // SubTexture
	}

	out << YAML::EndMap; // Texture

	std::ofstream stream(filepath);
	stream << out.c_str();
	stream.close();
}

void TextureSerializer::Deserialize(const std::string& filepath)
{
	std::ifstream stream(filepath);
	std::stringstream strStream;
	strStream << stream.rdbuf();

	YAML::Node textureRoot = YAML::Load(strStream.str());

	m_Texture.reset();
	m_SubTexture.reset();

	TextureProperties props = {(TextureFilter) textureRoot["Filter"].as<int>(), TextureWrap::Clamp };
	m_Texture = std::make_shared<Texture2D>(textureRoot["AssetPath"].as<std::string>(), props);

	if (textureRoot["SubTexture"]) {
		YAML::Node subTextureRoot = textureRoot["SubTexture"];
		//Check if valid subtexture
		if (subTextureRoot["HorizontalSpriteCount"] && subTextureRoot["VerticalSpriteCount"] && subTextureRoot["FrameRate"]) {
			//Create subtexture
			int horizontalCount = subTextureRoot["HorizontalSpriteCount"].as<int>();
			int verticalCount = subTextureRoot["VerticalSpriteCount"].as<int>();
			int framerate = subTextureRoot["FrameRate"].as<int>();
			m_SubTexture = std::make_shared<SubTexture2D>(m_Texture, horizontalCount, verticalCount, framerate);
		}
	}
}

Ref<Texture2D> TextureSerializer::GetTexture() {
	return m_Texture;
}

Ref<SubTexture2D> TextureSerializer::GetSubTexture() {
	return m_SubTexture;
}
