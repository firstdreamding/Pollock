#include "ParticleEditor.h"

#include "Pollock/ParticleSerializer.h"

ParticleEditor::ParticleEditor()
{
	 
}

ParticleEditor::~ParticleEditor()
{
}

void ParticleEditor::OnUpdate(float ts)
{
	for (int i = 0; i < m_ParticleInstances.size(); i++)
	{
		m_ParticleInstances[i].System->Emit(*m_ParticleInstances[i].Properties);
		m_ParticleInstances[i].System->OnUpdate(ts, i == m_index);
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
		for (int i = 0; i < m_ParticleInstances.size(); i++) {
			ImGui::PushID(i);
			if (ImGui::Button(m_ParticleInstances[i].System->Name)) {
				m_index = i;
			}
			ImGui::PopID();
		}
	}
	ImGui::End();

	ImGui::Begin("Properties");

	if (m_index != -1)
	{
		ImGui::InputText("Particle Name", m_ParticleInstances[m_index].System->Name, IM_ARRAYSIZE(m_ParticleInstances[m_index].System->Name));
		ImGui::DragFloat2("Position", glm::value_ptr(m_ParticleInstances[m_index].Properties->Position), 0.05f);

		ImGui::DragFloat2("Velocity", glm::value_ptr(m_ParticleInstances[m_index].Properties->Velocity), 0.05f);
		ImGui::DragFloat2("Velocity Variation", glm::value_ptr(m_ParticleInstances[m_index].Properties->VelocityVariation), 0.05f);

		ImGui::ColorEdit4("Birth Color", glm::value_ptr(m_ParticleInstances[m_index].Properties->BirthColor));
		ImGui::ColorEdit4("Death Color", glm::value_ptr(m_ParticleInstances[m_index].Properties->DeathColor));

		ImGui::DragFloat("Birth Size", &m_ParticleInstances[m_index].Properties->BirthSize, 0.02f);
		ImGui::DragFloat("Birth Size Variation", &m_ParticleInstances[m_index].Properties->BirthSizeVariation, 0.02f);
		ImGui::DragFloat("Death Size", &m_ParticleInstances[m_index].Properties->DeathSize, 0.02f);
		ImGui::DragFloat("Death Size Variation", &m_ParticleInstances[m_index].Properties->DeathSizeVariation, 0.02f);

		ImGui::DragFloat("Rotation Speed", &m_ParticleInstances[m_index].Properties->RotationSpeed);
		ImGui::DragFloat("Rotation Variation", &m_ParticleInstances[m_index].Properties->RotationVariation);
		ImGui::DragFloat("Rotation Speed Variation", &m_ParticleInstances[m_index].Properties->RotationSpeedVariation);

		ImGui::DragFloat("Life Span", &m_ParticleInstances[m_index].Properties->LifeSpan);
		ImGui::DragFloat("Life Span Variation", &m_ParticleInstances[m_index].Properties->LifeSpanVariation);
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

	if (ImGui::Button("Save"))
	{
		std::string filename = "ParticleEditorTest.particle";

		ParticleSerializer serializer;
		serializer.Serialize(filename, m_ParticleInstances);
	}


	ImGui::End();
}

void ParticleEditor::AddParticleSystem()
{
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

	m_ParticleInstances.push_back({ std::make_shared<ParticleSystem>(), std::make_shared<ParticleProperties>(defaultParticle) });
	m_index = m_ParticleInstances.size() - 1;
}
