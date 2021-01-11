#pragma once

#include "Pollock/Texture.h"
#include "Pollock/AnimationPlayer.h"

class TextureEditor
{
public:
	TextureEditor();
	~TextureEditor();

	void OnImGuiRender();

	Ref<Texture2D> GetCurrentTexture() const { return m_CurrentTexture; }
	Ref<SubTexture2D> GetAnimationTexture() const { return m_AnimationTexture; }
	Ref<AnimationPlayer> GetAnimationPlayer() const { return m_AnimationPlayer; }
private:
	Ref<Texture2D> m_CheckerboardTexture;
	Ref<Texture2D> m_CurrentTexture;
	bool m_IsAnimation = false;
	int m_VerticalSpriteCount = 1, m_HorizontalSpriteCount = 1;
	int m_Framerate = 15;

	Ref<SubTexture2D> m_AnimationTexture;
	Ref<AnimationPlayer> m_AnimationPlayer;
};
