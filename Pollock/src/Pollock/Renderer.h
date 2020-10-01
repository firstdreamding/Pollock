#pragma once

#include <glm/glm.hpp>

#include "Camera.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texCoord;
};

class Renderer
{
public:
	static void Init();
	static void OnWindowResize(uint32_t width, uint32_t height);

	static void SetCamera(const Camera& camera);
		
	static void Clear();
	
	static void SetWireframe(bool wireframe);

	static void Begin();
	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color);
	static void End();
};
