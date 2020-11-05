#pragma once

#include <string>
#include <memory>

#include "Window.h"
#include "Framebuffer.h"
#include "Camera.h"

#include <imgui.h>

#include "Base.h"
#include "ParticleSystem.h"

class Application
{
public:
	Application(const std::string& name, uint32_t width, uint32_t height);
	~Application();

	void Run();

	void SetOnUpdateCallback(const std::function<void(float)>& func) { m_OnUpdateCallback = func; }
	void SetImGuiDrawCallback(const std::function<void()>& func) { m_OnImGuiDrawCallback = func; }
	void SetPostViewportDrawCallback(const std::function<void(ImVec2)>& func) { m_OnPostViewportDrawCallback = func; }

	void OnImGuiRender();

	static bool IsKeyPressed(int keycode);

	Camera& GetCamera() { return m_Camera; }

	std::wstring OpenFile();
	std::wstring SaveFile();
private:
	void ResizeIfNeeded(uint32_t width, uint32_t height);
	void DrawViewport();
	void MoveCamera(Camera& camera, glm::vec2& position, const std::unique_ptr<Window>& window, float timestep);
	void SetCameraProjection(Camera& camera, uint32_t width, uint32_t height, float zoom);
private:
	std::string m_Name;
	uint32_t m_Width, m_Height;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	std::unique_ptr<Window> m_Window;
	std::unique_ptr<Framebuffer> m_Framebuffer;

	std::vector<std::function<void()>> m_PostRenderQueue;

	std::function<void(float)> m_OnUpdateCallback;
	std::function<void(ImVec2)> m_OnPostViewportDrawCallback;
	std::function<void()> m_OnImGuiDrawCallback;

	// TODO: Remove
	Camera m_Camera = Camera(-1.6f, 1.6f, -1.0f, 1.0f);
	glm::vec2 m_CameraPosition = { 0.0f, 0.0f };
	float m_CameraZoom = 1.0f;
	float m_Speed = 1.0f;
	ParticleProperties m_Particle;
};
