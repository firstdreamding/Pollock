#include "TextureEditor.h"

#include <imgui.h>
#include <iostream>
#include "Pollock/Utils/FileDialogs.h"

#include "Pollock/TextureSerializer.h"

TextureEditor::TextureEditor()
{
	TextureProperties props = { TextureFilter::Nearest, TextureWrap::Clamp };
	m_CheckerboardTexture = std::make_shared<Texture2D>("assets/editor/Checkerboard.tga", props);
}

TextureEditor::~TextureEditor()
{
}

void TextureEditor::OnImGuiRender()
{
	ImGui::Begin("Texture");

	if (m_CurrentTexture)
		ImGui::Image((ImTextureID)m_CurrentTexture->GetRendererID(), { 96, 96 }, { 0, 1 }, { 1, 0 });
	else
		ImGui::Image((ImTextureID)m_CheckerboardTexture->GetRendererID(), { 96, 96 }, { 0, 1 }, { 1, 0 });

	if (ImGui::IsItemHovered())
	{
		if (m_CurrentTexture)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(m_CurrentTexture->GetPath().c_str());
			ImGui::PopTextWrapPos();
			float aspectRatio = m_CurrentTexture->GetHeight() / (float)m_CurrentTexture->GetWidth();
			ImGui::Image((ImTextureID)m_CurrentTexture->GetRendererID(), { 512, 512 * aspectRatio }, { 0, 1 }, { 1, 0 });
			ImGui::EndTooltip();
		}

		if (ImGui::IsItemClicked())
		{
			std::string filepath = FileDialogs::OpenFile("Image Files(*.png, *.jpg, *.tga)\0 *.png;*.jpg;*.jpeg;*.tga\0");
			if (!filepath.empty())
			{
				TextureProperties props = { TextureFilter::Linear, TextureWrap::Clamp };
				m_CurrentTexture = std::make_shared<Texture2D>(filepath, props);
			}
		}
	}

	ImGui::SameLine();
	ImGui::BeginGroup();
	if (m_CurrentTexture)
	{
		ImGui::Text("%s", m_CurrentTexture->GetPath().c_str());
		ImGui::Text("W: %d", m_CurrentTexture->GetWidth());
		ImGui::Text("H: %d", m_CurrentTexture->GetHeight());
	}
	ImGui::EndGroup();

	if (m_CurrentTexture)
	{
		ImGui::Separator();

		const char* filteringTypeStrings[] = { "Linear", "Nearest" };
		static const char* currentFilterType = filteringTypeStrings[0];
		if (ImGui::BeginCombo("Filtering", currentFilterType))
		{
			for (int type = 0; type < 2; type++)
			{
				bool isSelected = (currentFilterType == filteringTypeStrings[type]);
				if (ImGui::Selectable(filteringTypeStrings[type], isSelected))
				{
					currentFilterType = filteringTypeStrings[type];
					// set texture filtering
					m_CurrentTexture->SetFilter((TextureFilter)(type + 1));
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();

		ImGui::Checkbox("Animation", &m_IsAnimation);
		if (m_IsAnimation)
		{
			ImGui::DragInt("Framerate", &m_Framerate);
			ImGui::DragInt("Vertical Sprite Count", &m_VerticalSpriteCount);
			ImGui::DragInt("Horizontal Sprite Count", &m_HorizontalSpriteCount);
			if (ImGui::Button("Create Sub-Texture"))
			{
				m_AnimationTexture = std::make_shared<SubTexture2D>(m_CurrentTexture, m_VerticalSpriteCount, m_HorizontalSpriteCount);
			}
		}

		if (ImGui::Button("Save As..."))
		{
			std::string filepath = FileDialogs::SaveFile("Pollock Texture(*.ptex)\0 *.ptex\0");
			if (!filepath.empty())
			{
				TextureSerializer serializer(m_CurrentTexture, m_AnimationTexture);
				serializer.Serialize(filepath);
			}
		}

	}
	ImGui::End();
}