#pragma once

#include "Pollock/Texture.h"

class TextureEditor
{
public:
	TextureEditor();
	~TextureEditor();

	void OnImGuiRender();

	Ref<Texture2D> GetCurrentTexture() const { return m_CurrentTexture; }
	Ref<SubTexture2D> GetAnimationTexture() const { return m_AnimationTexture; }
private:
	Ref<Texture2D> m_CheckerboardTexture;
	Ref<Texture2D> m_CurrentTexture;
	bool m_IsAnimation = false;
	int m_VerticalSpriteCount = 0, m_HorizontalSpriteCount = 0;

	Ref<SubTexture2D> m_AnimationTexture;
};
