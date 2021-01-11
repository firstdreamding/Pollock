#pragma once

#include <memory>
#include <string>
#include <stdint.h>
#include <functional>

#include <glm/glm.hpp>

#include "KeyCodes.h"

#define NOMINMAX
#include <Windows.h>

struct GLFWwindow;

class Window
{
public:
	Window(const std::string& name, uint32_t width, uint32_t height);
	~Window();

	void Create(const std::string& name, uint32_t width, uint32_t height);
	void SetResizeCallback(const std::function<void(uint32_t, uint32_t)>& func) { m_ResizeCallback = func; }
	void SetMouseScrollCallback(const std::function<void(float, float)>& func) { m_MouseScrollCallback = func; }
	void SetOnImGuiRenderCallback(const std::function<void()>& func) { m_ImGuiOnRenderCallback = func; }

	void OnImGuiRender();
	void Update();

	bool IsKeyPressed(int keycode) const;

	bool IsMousePressed(int button) const;

	bool IsMouseReleased(int button) const;

	glm::vec2 GetMousePosition() const;

	bool IsClosed() const;

	HWND GetWin32Window();
private:
	void Create();
private:
	std::string m_Name;
	uint32_t m_Width, m_Height;

	std::function<void(uint32_t, uint32_t)> m_ResizeCallback;
	std::function<void(float, float)> m_MouseScrollCallback;
	std::function<void()> m_ImGuiOnRenderCallback;

	GLFWwindow* m_WindowHandle = nullptr;
};
