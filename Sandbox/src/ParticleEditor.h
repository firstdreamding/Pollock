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

#include "Pollock/Camera.h"
#include <fstream>


class ParticleEditor
{
public:
	ParticleEditor();
	~ParticleEditor();

	void LoadFile(const std::wstring& filename);

	void OnUpdate(float ts);
	void OnImGuiDraw();
	void DrawGizmo(const Camera& camera, ImVec2 viewportSize);
	void OnMenuImGuiDraw();

	void AddParticleSystem();

	void SetAnimation(Ref<AnimationPlayer> animation);
private:
	std::vector<ParticleInstance> m_ParticleInstances;
	Ref<Texture2D> m_CheckerboardTexture;

	int m_index = -1;
	int m_ActiveGizmo = -1;
	bool m_ShowWireframe = true;
	float m_seconds = 0;	 
};