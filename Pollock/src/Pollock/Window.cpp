#include "Window.h"

#include <iostream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Renderer.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

Window::Window(const std::string& name, uint32_t width, uint32_t height)
	: m_Name(name), m_Width(width), m_Height(height)
{
	Create();
}

Window::~Window()
{
	glfwTerminate();
}

void Window::Create()
{
	/* Initialize the library */
	if (!glfwInit())
		return;

	/* Create a windowed mode window and its OpenGL context */
	m_WindowHandle = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), NULL, NULL);
	if (!m_WindowHandle)
	{
		glfwTerminate();
		return;
	}

	glfwSetWindowUserPointer(m_WindowHandle, this);

	glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
	{
		Renderer::OnWindowResize(width, height);

		Window* win = (Window*)glfwGetWindowUserPointer(window); 

		if (win->m_ResizeCallback)
			win->m_ResizeCallback(width, height);
	});

	glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double x, double y)
	{
		Window* win = (Window*)glfwGetWindowUserPointer(window);

		if (win->m_MouseScrollCallback)
			win->m_MouseScrollCallback(x, y);
	});

	/* Make the window's context current */
	glfwMakeContextCurrent(m_WindowHandle);
	glfwSwapInterval(0);

	if (!gladLoadGL())
	{
		std::cout << "Failed to load glad!\n";
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImFont* pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	io.FontDefault = io.Fonts->Fonts.back();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(m_WindowHandle, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
}

void Window::Create(const std::string& name, uint32_t width, uint32_t height)
{
	m_Name = name;
	m_Width = width;
	m_Height = height;
	Create();
}

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void Window::Update()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	if (m_ImGuiOnRenderCallback)
		m_ImGuiOnRenderCallback();

	// ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	glfwSwapBuffers(m_WindowHandle);
	glfwPollEvents();
}

bool Window::IsKeyPressed(int keycode) const
{
	return glfwGetKey(m_WindowHandle, keycode) == GLFW_PRESS;
}

bool Window::IsMousePressed(int button) const
{
	return glfwGetMouseButton(m_WindowHandle, button) == GLFW_PRESS;
}

bool Window::IsMouseReleased(int button) const
{
	return glfwGetMouseButton(m_WindowHandle, button) == GLFW_RELEASE;
}

glm::vec2 Window::GetMousePosition() const
{
	double xpos, ypos;
	glfwGetCursorPos(m_WindowHandle, &xpos, &ypos);
	glm::vec2 return_vec; 
	return_vec.x = xpos;
	return_vec.y = ypos;
	return return_vec;
}

bool Window::IsClosed() const
{
	return glfwWindowShouldClose(m_WindowHandle);
}

HWND Window::GetWin32Window()
{
	return glfwGetWin32Window(m_WindowHandle);
}
