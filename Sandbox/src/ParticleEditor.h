#pragma once

#include <vector>
#include "Pollock/ParticleSystem.h"


#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include <Pollock/Log.h>
#include <Pollock/Window.h>

#include <iostream>

#include <chrono>

#include "imgui.h"

#include <fstream>


class ParticleEditor
{
public:
	ParticleEditor();
	~ParticleEditor();

	void OnUpdate(float ts);
	void OnImGuiDraw();
	void OnMenuImGuiDraw();

	void AddParticleSystem();

	std::wstring ParticleEditor::SaveFile();
	std::wstring ParticleEditor::OpenFile();

private:
	std::vector<ParticleInstance> m_ParticleInstances;

	int m_index = -1;
};