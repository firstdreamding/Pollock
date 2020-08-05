#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	Camera(float left, float right, float bottom, float top);

	void SetProjection(const glm::mat4& projection) { m_ProjectionMatrix = projection; RecalcViewProj(); }
	void SetTranslation(const glm::vec2& translation);

	const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	const glm::mat4& GetViewProj() const { return m_ViewProjectionMatrix; }
private:
	void RecalcViewProj();
private:
	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ViewProjectionMatrix;
};