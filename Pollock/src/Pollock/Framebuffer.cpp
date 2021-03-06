#include "Framebuffer.h"

#include "Renderer.h"

#include "glad/glad.h"

#include <iostream>

Framebuffer::Framebuffer(uint32_t width, uint32_t height)
	: m_Width(width), m_Height(height)
{
	Invalidate();
}

Framebuffer::~Framebuffer()
{
	uint32_t rendererID = m_RendererID;
	uint32_t colorBufferRendererID = m_ColorBufferRendererID;
	uint32_t depthBufferRendererID = m_DepthBufferRendererID;
	Renderer::Submit([rendererID, colorBufferRendererID, depthBufferRendererID]()
	{
		glDeleteFramebuffers(1, &rendererID);
		glDeleteTextures(1, &colorBufferRendererID);
		glDeleteTextures(1, &depthBufferRendererID);
	});
}

void Framebuffer::Bind()
{
	Ref<Framebuffer> instance = this;
	Renderer::Submit([instance]() mutable
	{
		glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
		glViewport(0, 0, instance->m_Width, instance->m_Height);
	});
}

void Framebuffer::Unbind()
{
	Ref<Framebuffer> instance = this;
	Renderer::Submit([instance]() mutable
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}

void Framebuffer::Resize(uint32_t width, uint32_t height)
{
	if (m_Width == width && m_Height == height)
		return;

	m_Width = width;
	m_Height = height;
	Invalidate();
}

void Framebuffer::Invalidate()
{
	Ref<Framebuffer> instance = this;
	Renderer::Submit([instance]() mutable
	{
		instance->RT_Invalidate();
	});
}

void Framebuffer::RT_Invalidate()
{
	if (m_RendererID)
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_ColorBufferRendererID);
		glDeleteTextures(1, &m_DepthBufferRendererID);
	}

	glCreateFramebuffers(1, &m_RendererID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorBufferRendererID);
	glBindTexture(GL_TEXTURE_2D, m_ColorBufferRendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorBufferRendererID, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthBufferRendererID);
	glBindTexture(GL_TEXTURE_2D, m_DepthBufferRendererID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Width, m_Height);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthBufferRendererID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer incomplete!\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
