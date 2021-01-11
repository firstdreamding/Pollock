#include "ApplicationSettings.h"

ApplicationSettings::ApplicationSettings(ApplicationProperties& applicationProp)
{
	m_ApplicationProp = &applicationProp;
}

void ApplicationSettings::OnImGuiDraw()
{
	ImGui::Begin("Application Settings");
	ImGui::ColorEdit4("Birth Color", glm::value_ptr(m_ApplicationProp->m_BackgroundColor));
	ImGui::End();
}
