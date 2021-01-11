#pragma once

#include "Texture.h"

class AnimationPlayer
{
public:
	explicit AnimationPlayer(const Ref<SubTexture2D>& animationTexture);
	AnimationPlayer(const AnimationPlayer&) = default;

	void OnUpdate(float ts);

	Ref<Texture2D> GetTexture() const { return m_AnimationTexture->GetTexture(); }
	const glm::vec2* GetTextureCoords() const;

	float GetFrameRate() const { return m_FrameRate; }
	void SetFrameRate(float framerate);
private:
	Ref<SubTexture2D> m_AnimationTexture;
	float m_FrameRate;

	float m_Timer = 0.0f;
	float m_AnimationTimer = 0.0f;
	uint32_t m_CurrentFrame = 0;
};
