#include "TextureSerializer.h"

#include "yaml-cpp/yaml.h"

#include <fstream>

TextureSerializer::TextureSerializer(Ref<Texture2D> texture, Ref<SubTexture2D> subTexture)
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

}
