#pragma once

#include <string>
#include <memory>

#include "Window.h"
#include "Framebuffer.h"
#include "Camera.h"

#include <imgui.h>
#include <thread>

#include "Base.h"
#include "ParticleSystem.h"

struct ApplicationProperties
{
	glm::vec4 m_BackgroundColor = { 0.1f, 0.1f, 0.1f, 1.0f };
};

class Application
{
public:
	Application(const std::string& name, uint32_t width, uint32_t height);
	~Application();

	static Application* Get();
	
	void Run();
	void Shutdown();

	void SetOnUpdateCallback(const std::function<void(float)>& func) { m_OnUpdateCallback = func; }
	void SetImGuiDrawCallback(const std::function<void()>& func) { m_OnImGuiDrawCallback = func; }
	void SetPostViewportDrawCallback(const std::function<void(ImVec2)>& func) { m_OnPostViewportDrawCallback = func; }

	void OnImGuiRender();
	
	bool IsRunning() const { return m_IsRunning; }
	
	static bool IsKeyPressed(int keycode);

	Camera& GetCamera() { return m_Camera; }

	std::wstring OpenFile();
	std::wstring SaveFile();

	template<typename Fn>
	std::thread& CreateThread(Fn&& func) // r-value reference
	{
		return m_ThreadPool.emplace_back(std::forward<Fn>(func));
	}

	ApplicationProperties m_ApplicationProp;
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

	std::vector<std::thread> m_ThreadPool;
	
	bool m_IsRunning = false;

	// TODO: Remove
	bool m_FirstClick = true;
	Camera m_Camera = Camera(-1.6f, 1.6f, -1.0f, 1.0f);
	glm::vec2 m_CameraPosition = { 0.0f, 0.0f };
	glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };
	glm::vec2 m_LastCameraPosition = { 0.0f, 0.0f };
	glm::vec2 m_CameraSpeed = { 0.0f, 0.0f };
	float m_CameraZoom = 1.0f;
	float m_Speed = 1.0f;
	ParticleProperties m_Particle;
};
