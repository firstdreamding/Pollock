#pragma once

#include <vector>
#include "Pollock/ParticleSystem.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<string>

class ParticleEditor
{
public:
	ParticleEditor();
	~ParticleEditor();

	void OnUpdate(float ts);
	void OnImGuiDraw();

	void AddParticleSystem();
private:
	std::vector<std::shared_ptr<ParticleSystem>> m_ParticleSystems;
	std::vector<std::shared_ptr<ParticleProperties>> m_Particles;
	int m_index = -1;
};