#pragma once

#include <glm/glm.hpp>

#include <functional>

#include "Camera.h"
#include "Texture.h"

#include "ResourceQueue.h"
#include "Timer.h"
#include "Utils/Instrumentor.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texCoord;
	float textureID;
};

class Renderer
{
public:
	using RenderCommandFn = std::function<void()>;

	static void Submit(const RenderCommandFn& func)
	{
		auto& queue = s_RenderCommandQueue[s_RenderCommandQueueSubmissionIndex];
		queue.emplace_back(func);
	}

	static void SwapQueues()
	{
		s_RenderCommandQueueSubmissionIndex = (s_RenderCommandQueueSubmissionIndex + 1) % s_RenderCommandQueueCount;
	}

	static uint32_t GetRenderQueueIndex()
	{
		return (s_RenderCommandQueueSubmissionIndex + 1) % s_RenderCommandQueueCount;
	}

	static void ExecuteRenderCommandQueue();

	static void Init();
	static void OnWindowResize(uint32_t width, uint32_t height);

	static void SetCamera(const Camera& camera);
		
	static void Clear(glm::vec4 color);
	
	static void SetWireframe(bool wireframe);

	static void ProcessResources();

	static ResourceQueue& GetResourceQueue();

	static void Begin();
	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color);
	static void DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	static void DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec2* texCoords, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	static void DrawRotatedTexturedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, Texture2D* texture, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	static void DrawRotatedTexturedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, Texture2D* texture, const glm::vec2* texCoords, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	static void End();
private:
	constexpr static uint32_t s_RenderCommandQueueCount = 2;
	inline static std::vector<RenderCommandFn> s_RenderCommandQueue[s_RenderCommandQueueCount];
	inline static std::atomic<uint32_t> s_RenderCommandQueueSubmissionIndex = 0;
};
