#include "AnimationPlayer.h"

AnimationPlayer::AnimationPlayer(const Ref<SubTexture2D>& animationTexture)
	: m_AnimationTexture(animationTexture)
{
	m_FrameRate = animationTexture->GetFrameRate();
	m_AnimationTimer = 1.0f / (float)m_FrameRate;
}

void AnimationPlayer::OnUpdate(float ts)
{
	m_Timer -= ts;
	if (m_Timer <= 0.0f && m_AnimationTimer > 0.0f)
	{
		m_Timer = m_AnimationTimer;
		m_CurrentFrame++;
	}

	int totalFrames = m_AnimationTexture->GetHorizontalSpriteCount() * m_AnimationTexture->GetVerticalSpriteCount();
	m_CurrentFrame %= totalFrames;
}

const glm::vec2* AnimationPlayer::GetTextureCoords() const
{
	int xc = m_CurrentFrame % m_AnimationTexture->GetHorizontalSpriteCount();
	int yc = m_CurrentFrame / m_AnimationTexture->GetHorizontalSpriteCount();
	// assert
	return m_AnimationTexture->GetTextureCoords(xc, yc);
}

void AnimationPlayer::SetFrameRate(float framerate)
{
	m_FrameRate = framerate;
	m_AnimationTimer = framerate > 0.0f ? 1.0f / (float)framerate : 0.0f;
}

