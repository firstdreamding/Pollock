#include "ParticleEditor.h"

#include "Pollock/ParticleSerializer.h"
#include "ImGuizmo.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "Pollock/Application.h"

#include "Pollock/Utils/FileDialogs.h"

ParticleEditor::ParticleEditor()
{
}

ParticleEditor::~ParticleEditor()
{
}

void ParticleEditor::LoadFile(const std::wstring& filename)
{
	ParticleSerializer serializer;
	serializer.Deserialize(filename, m_ParticleInstances);
}

void ParticleEditor::OnUpdate(float ts)
{
	// Keyboard shortcuts
	if (Application::IsKeyPressed(PL_KEY_Q))
		m_ActiveGizmo = -1;
	if (Application::IsKeyPressed(PL_KEY_W))
		m_ActiveGizmo = ImGuizmo::TRANSLATE;
	if (Application::IsKeyPressed(PL_KEY_E))
		m_ActiveGizmo = ImGuizmo::ROTATE;

	// Particles
	for (int i = 0; i < m_ParticleInstances.size(); i++)
	{
		m_ParticleInstances[i].System->Emit(*m_ParticleInstances[i].Properties, ts);
		m_ParticleInstances[i].System->OnUpdate(ts, m_ShowWireframe && (i == m_index));
	}
}

void ParticleEditor::OnImGuiDraw()
{
	OnMenuImGuiDraw();


	ImGui::Begin("Controls");
	if (ImGui::Button("Add Particle System"))
	{
		AddParticleSystem();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Show selection wireframe", &m_ShowWireframe);
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
		if (m_ParticleInstances[m_index].Properties->Texture)
			ImGui::Image((ImTextureID)m_ParticleInstances[m_index].Properties->Texture->GetRendererID(), { 96, 96 }, { 0, 1 }, { 1, 0 });
		else {
			if (!m_CheckerboardTexture) {
				TextureProperties props = { TextureFilter::Nearest, TextureWrap::Clamp };
				m_CheckerboardTexture = std::make_shared<Texture2D>("assets/editor/Checkerboard.tga", props);
			}
			ImGui::Image((ImTextureID)m_CheckerboardTexture->GetRendererID(), { 96, 96 }, { 0, 1 }, { 1, 0 });
		}

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

		ImGui::DragInt("Emission Count", (int*)&m_ParticleInstances[m_index].Properties->EmissionCount, 1, 1, 1000);
		ImGui::DragFloat2("Position", glm::value_ptr(m_ParticleInstances[m_index].Properties->Position), 0.05f);

		ImGui::DragFloat("Emission Angle", &m_ParticleInstances[m_index].Properties->EmissionAngle, 0.05f);
		ImGui::DragFloat("Emission Force", &m_ParticleInstances[m_index].Properties->EmissionForce, 0.05f);
		ImGui::DragFloat("Emission Angle Variation", &m_ParticleInstances[m_index].Properties->EmissionAngleVariation, 0.05f);
		ImGui::DragFloat("Emission Force Variation", &m_ParticleInstances[m_index].Properties->EmissionForceVariation, 0.05f);

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

		if (ImGui::Button("Delete")) {
			m_ParticleInstances.erase(m_ParticleInstances.begin() + m_index);
			m_index = -1;
		}
		ImGui::PopItemWidth();
	}

	//ImGui::DragInt("Emission Rate", &s_EmissionRate, 1, 0, 100);
	//ImGui::SliderFloat("Speed", &s_Speed, 0.1f, 5.0f);

	//TODO:: See if can implement popup :)
	/*
	
	if (ImGui::Button("Popup")) {
		ImGui::OpenPopup("Importing Particles");
	}

	if (ImGui::BeginPopupModal("Importing Particles"))
	{
		ImGui::PushTextWrapPos(500);
		ImGui::Text("Would you like to merge this file with the existing particles in this workspace or replace the workspace with the particles within the contents of the file? (Choosing the latter option will delete all unsaved progress made in your workspace)", ImVec2(480, 200));
		ImGui::Spacing();
		if (ImGui::Button("Merge", ImVec2(240, 40))) { ImGui::CloseCurrentPopup(); }
		ImGui::SameLine();
		if (ImGui:: Button("Replace", ImVec2(240, 40))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	*/
	ImGui::End();
}

static glm::quat GetRotationFromMatrix(const glm::mat4& matrix)
{
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, orientation, translation, skew, perspective);

	return orientation;
}

void ParticleEditor::DrawGizmo(const Camera& camera, ImVec2 viewportSize)
{
	if (m_index == -1 || m_ActiveGizmo == -1)
		return;

	auto& particleInstance = m_ParticleInstances[m_index];
	auto& position = particleInstance.Properties->Position;

	// particle properties
	//   - angle (float)
	//   - force (float) 0->100
	//   - vec2(x, y)

	float angle = glm::radians(particleInstance.Properties->EmissionAngle);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
		* glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1));

	glm::mat4 oldTransform = transform;

	ImGuizmo::SetOrthographic(true);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewportSize.x, viewportSize.y);

	glm::mat4 view = camera.GetViewMatrix();
	ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(camera.GetProjectionMatrix()),
		(ImGuizmo::OPERATION)m_ActiveGizmo, ImGuizmo::LOCAL, glm::value_ptr(transform));

	if (transform != oldTransform)
	{
		glm::quat rotation = GetRotationFromMatrix(transform);
		glm::vec3 rotationEuler = glm::eulerAngles(rotation);

		particleInstance.Properties->EmissionAngle = glm::degrees(rotationEuler.z);
		position.x = transform[3][0];
		position.y = transform[3][1];
	}

}

void ParticleEditor::OnMenuImGuiDraw() {
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save All")) {
				ParticleSerializer serializer;
				serializer.Serialize(FileDialogs::SaveFileW(L"Particle Files (*.particle)\0*.particle\0"), m_ParticleInstances);
			}
			if (ImGui::MenuItem("Save Selected")) {
				ParticleSerializer serializer;
				serializer.Serialize(FileDialogs::SaveFileW(L"Particle Files (*.particle)\0*.particle\0"), m_ParticleInstances);
			}
			if (ImGui::MenuItem("Open")) {
				std::wstring filepath = FileDialogs::OpenFileW(L"Particle Files (*.particle)\0*.particle\0");
				if (!filepath.empty())
					LoadFile(filepath);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void ParticleEditor::AddParticleSystem()
{
	ParticleProperties defaultParticle;
	defaultParticle.Position = { 0.0f, 0.0f };
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

void ParticleEditor::SetAnimation(Ref<AnimationPlayer> animation)
{
	if (m_ParticleInstances.size())
	{
		m_ParticleInstances[0].Properties->Texture = animation->GetTexture().get();
		m_ParticleInstances[0].Properties->Animation = animation;
	}
}
