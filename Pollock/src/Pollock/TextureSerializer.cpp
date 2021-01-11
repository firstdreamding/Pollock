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


	YAML::Node textureData = YAML::Load(strStream.str());

	if (textureData["AssetPath"]) {
		Texture2D instanceTexture(textureData["AssetPath"].as<std::string>());
		if (textureData["Filter"])
			instanceTexture.SetFilter((TextureFilter)textureData["Filter"].as<int>());
		m_Texture = std::make_shared<Texture2D>(instanceTexture);
	}

	if (textureData["SubTexture"]) {
		YAML::Node subTextureData = textureData["SubTexture"];
		if (subTextureData["HorizontalSpriteCount"] && subTextureData["VerticalSpriteCount"]) {
			SubTexture2D instanceSubTexture(m_Texture,
											subTextureData["HorizontalSpriteCount"].as<int>(),
											subTextureData["VerticalSpriteCount"].as<int>());

			m_SubTexture = std::make_shared<SubTexture2D>(instanceSubTexture);
		}
	}
	
}
