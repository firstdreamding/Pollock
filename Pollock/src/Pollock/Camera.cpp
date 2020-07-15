#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float left, float right, float bottom, float top)
{
	m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
	m_ViewMatrix = glm::mat4(1.0f);
	RecalcViewProj();
}

void Camera::SetTranslation(const glm::vec2& translation)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { translation.x, translation.y, 0.0f });
	m_ViewMatrix = glm::inverse(transform);
	RecalcViewProj();
}

void Camera::RecalcViewProj()
{
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}
