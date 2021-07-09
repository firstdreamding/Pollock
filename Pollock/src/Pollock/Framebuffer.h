#pragma once

#include <stdint.h>
#include "Base.h"

class Framebuffer : public RefCounted
{
public:
	Framebuffer(uint32_t width, uint32_t height);
	~Framebuffer();

	void Bind();
	void Unbind();

	void Resize(uint32_t width, uint32_t height);

	uint32_t GetColorBufferRendererID() const { return m_ColorBufferRendererID; }
private:
	uint32_t m_Width, m_Height;
	uint32_t m_RendererID = 0;
	uint32_t m_ColorBufferRendererID, m_DepthBufferRendererID;

	void Invalidate();
	void RT_Invalidate();
};
