#pragma once
#include <Pollock/Application.h>
#include <glm/gtc/type_ptr.hpp>

class ApplicationSettings
{
public:
	ApplicationSettings(ApplicationProperties& applicationProp);

	void OnImGuiDraw();

private:
	int m_index = -1;
	int m_ActiveGizmo = -1;
	bool m_ShowWireframe = true;
	ApplicationProperties* m_ApplicationProp;
};