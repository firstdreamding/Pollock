#include "ParticleEditor.h"

ParticleEditor::ParticleEditor()
{
	 
}

ParticleEditor::~ParticleEditor()
{
}

void ParticleEditor::OnUpdate(float ts)
{
	for (int i = 0; i < m_ParticleSystems.size(); i++)
	{
		m_ParticleSystems[i]->Emit(*m_Particles[i]);
		m_ParticleSystems[i]->OnUpdate(ts, i == m_index);
	}
}

void ParticleEditor::OnImGuiDraw()
{
	ImGui::Begin("Controls");
	if (ImGui::Button("Add Particle System"))
	{
		AddParticleSystem();
	}

	ImGui::SameLine();
	if (ImGui::Button("Deselect"))
		m_index = -1;
	if (ImGui::CollapsingHeader("Active Particle Systems"))
	{
		ImGui::Indent();
		for (int i = 0; i < m_ParticleSystems.size(); i++) {
			ImGui::PushID(i);
			if (ImGui::Button(m_ParticleSystems[i]->Name)) {
				m_index = i;
			}
			ImGui::PopID();
		}
	}
	ImGui::End();

	ImGui::Begin("Properties");

	if (m_index != -1)
	{
		ImGui::InputText("Particle Name", m_ParticleSystems[m_index]->Name, IM_ARRAYSIZE(m_ParticleSystems[m_index]->Name));
		ImGui::DragFloat2("Position", glm::value_ptr(m_Particles[m_index]->Position), 0.05f);

		ImGui::DragFloat2("Velocity", glm::value_ptr(m_Particles[m_index]->Velocity), 0.05f);
		ImGui::DragFloat2("Velocity Variation", glm::value_ptr(m_Particles[m_index]->VelocityVariation), 0.05f);

		ImGui::ColorEdit4("Birth Color", glm::value_ptr(m_Particles[m_index]->BirthColor));
		ImGui::ColorEdit4("Death Color", glm::value_ptr(m_Particles[m_index]->DeathColor));

		ImGui::DragFloat("Birth Size", &m_Particles[m_index]->BirthSize, 0.02f);
		ImGui::DragFloat("Birth Size Variation", &m_Particles[m_index]->BirthSizeVariation, 0.02f);
		ImGui::DragFloat("Death Size", &m_Particles[m_index]->DeathSize, 0.02f);
		ImGui::DragFloat("Death Size Variation", &m_Particles[m_index]->DeathSizeVariation, 0.02f);

		ImGui::DragFloat("Rotation Speed", &m_Particles[m_index]->RotationSpeed);
		ImGui::DragFloat("Rotation Variation", &m_Particles[m_index]->RotationVariation);
		ImGui::DragFloat("Rotation Speed Variation", &m_Particles[m_index]->RotationSpeedVariation);

		ImGui::DragFloat("Life Span", &m_Particles[m_index]->LifeSpan);
		ImGui::DragFloat("Life Span Variation", &m_Particles[m_index]->LifeSpanVariation);

	}

	//ImGui::DragInt("Emission Rate", &s_EmissionRate, 1, 0, 100);
	//ImGui::SliderFloat("Speed", &s_Speed, 0.1f, 5.0f);

	/*
	if (ImGui::Button("Save"))
	{
		auto filename = Sandbox::SaveFile();
		WriteParticleDataText(filename, particle);
	}

	if (ImGui::Button("Load"))
	{
		std::wstring string = OpenFile();
		if (!string.empty())
			m_Particles[m_index] = ReadParticleDataText(string);
	}
	*/

	ImGui::End();
}

void ParticleEditor::AddParticleSystem()
{
	m_ParticleSystems.push_back(std::make_shared<ParticleSystem>());

	ParticleProperties defaultParticle;
	defaultParticle.Position = { 0.0f, 0.0f };
	defaultParticle.Velocity = { 0.0f, 4.0f };
	defaultParticle.VelocityVariation = { 4.0f, 2.0f };
	defaultParticle.BirthColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	defaultParticle.DeathColor = { 1.0f, 1.0f, 0.0f, 0.0f };
	defaultParticle.BirthSize = 0.2f;
	defaultParticle.BirthSizeVariation = 0.1f;
	defaultParticle.DeathSize = 0.02f;
	defaultParticle.DeathSizeVariation = 0.04f;
	defaultParticle.RotationSpeed = 360.0f;
	defaultParticle.RotationVariation = 360.0f;
	defaultParticle.RotationSpeedVariation = 100.0f;
	defaultParticle.LifeSpan = 1.0f;
	defaultParticle.LifeSpanVariation = 1.0f;

	m_Particles.push_back(std::make_shared<ParticleProperties>(defaultParticle));
	m_index = m_Particles.size() - 1;
}
