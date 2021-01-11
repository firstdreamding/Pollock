#include "Application.h"

#include <chrono>

#include "Renderer.h"
#include "Random.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Windows.h>

#include "Texture.h"

static int s_EmissionRate = 5;

struct Stats
{
	float FPS = 0.0f;
	int Draws = 0;
};

static Stats s_Stats;

static Application* s_ActiveApplication = nullptr;

Application::Application(const std::string& name, uint32_t width, uint32_t height)
	: m_Name(name), m_Width(width), m_Height(height)
{
	s_ActiveApplication = this;

	m_Window = std::make_unique<Window>(name.c_str(), width, height);

	m_Window->SetResizeCallback([&](uint32_t width, uint32_t height)
	{
		// cameraWidth = width;
		// cameraHeight = height;
		// SetCameraProjection(camera, width, height, cameraZoom);
	});

	m_Window->SetMouseScrollCallback([&](float x, float y)
	{
		m_CameraZoom -= y * 0.5f;
		m_CameraZoom = glm::max(0.5f, m_CameraZoom);
		SetCameraProjection(m_Camera, m_ViewportWidth, m_ViewportHeight, m_CameraZoom);
	});

	
	m_Window->SetOnImGuiRenderCallback([&]() { OnImGuiRender(); });

	Renderer::Init();
	Random::Init();

	m_Framebuffer = std::make_unique<Framebuffer>(width, height);
}

Application::~Application()
{
}

void Application::Run()
{
	auto lastTime = std::chrono::high_resolution_clock::now();
	float timerValue = 1.0f;
	float timer = timerValue;
	int counter = 0;

	while (!m_Window->IsClosed())
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto difference = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastTime);
		lastTime = now;
		auto seconds = difference.count() * 0.001f * 0.001f * 0.001f; // 0.033
		timer -= seconds;

		MoveCamera(m_Camera, m_CameraPosition, m_Window, seconds);

		// Rendering
		m_Framebuffer->Bind();
		Renderer::Clear();
		Renderer::SetCamera(m_Camera);
		uint32_t quadCount = 0;

		if (m_OnUpdateCallback)
			m_OnUpdateCallback(seconds);

		// s_ParticleEditor.OnUpdate(seconds);
		// particleSystem.OnUpdate(seconds * s_Speed);

		/*for (int i = 0; i < s_EmissionRate; i++)
			particleSystem.Emit(particle);

		for (int i = 0; i < s_EmissionRate; i++)
			particleSystem.Emit(p);*/

		s_Stats.Draws = quadCount;

		if (timer <= 0.0f)
		{
			timer = timerValue;
			// OncePerSecond(counter);
			s_Stats.FPS = (1.0f / seconds);
		}
		//Renderer::DrawQuad({ -0.5f,  0.5f }, { 0.8f, 0.2f, 0.0f });
		//Renderer::DrawQuad({  0.5f,  0.5f }, { 0.2f, 0.2f, 0.8f });
		//Renderer::DrawQuad({ -0.5f, -0.5f }, { 0.8f, 0.2f, 0.8f });

		m_Framebuffer->Unbind();
		m_Window->Update();

		for (auto& func : m_PostRenderQueue)
			func();

		m_PostRenderQueue.clear();
	}
}


void Application::SetCameraProjection(Camera& camera, uint32_t width, uint32_t height, float zoom)
{
	float aspectRatio = (float)width / (float)height; // 1.78
	float multiplier = zoom;

	glm::mat4 projection = glm::ortho(-aspectRatio * multiplier, aspectRatio * multiplier, -1.0f * multiplier, 1.0f * multiplier);
	camera.SetProjection(projection);
}

void Application::MoveCamera(Camera& camera, glm::vec2& position, const std::unique_ptr<Window>& window, float timestep)
{
	float speed = 20.0f * timestep;

	if (window->IsKeyPressed(PL_KEY_UP))
		position.y += speed;
	else if (window->IsKeyPressed(PL_KEY_DOWN))
		position.y -= speed;

	if (window->IsKeyPressed(PL_KEY_LEFT))
		position.x -= speed;
	else if (window->IsKeyPressed(PL_KEY_RIGHT))
		position.x += speed;

	camera.SetTranslation(position);
}

static void DrawImGuiStatsPanel()
{
	ImGui::Begin("Stats");
	ImGui::Text("FPS: %.2f", s_Stats.FPS);
	ImGui::Text("Draws: %d", s_Stats.Draws);
	ImGui::End();
}

bool s_NeedsResize = false;

void Application::ResizeIfNeeded(uint32_t width, uint32_t height)
{
	if (m_ViewportWidth == width && m_ViewportHeight == height)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	m_PostRenderQueue.push_back([&]()
	{
		m_Framebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
		SetCameraProjection(m_Camera, m_ViewportWidth, m_ViewportHeight, m_CameraZoom);
	});
}

void Application::DrawViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::Begin("Viewport");
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	ImGui::Image((void*)m_Framebuffer->GetColorBufferRendererID(), viewportSize, { 0, 1 }, { 1, 0 });
	if (m_OnPostViewportDrawCallback)
		m_OnPostViewportDrawCallback(viewportSize);
	ResizeIfNeeded(viewportSize.x, viewportSize.y);
	ImGui::End();
	ImGui::PopStyleVar();
}

void Application::OnImGuiRender()
{
#if 0
	ImGui::Begin("Properties");
	ImGui::DragFloat2("Position", glm::value_ptr(particle.Position), 0.05f);

	ImGui::DragFloat2("Velocity", glm::value_ptr(particle.Velocity), 0.05f);
	ImGui::DragFloat2("Velocity Variation", glm::value_ptr(particle.VelocityVariation), 0.05f);

	ImGui::ColorEdit4("Birth Color", glm::value_ptr(particle.BirthColor));
	ImGui::ColorEdit4("Death Color", glm::value_ptr(particle.DeathColor));

	ImGui::DragFloat("Birth Size", &particle.BirthSize, 0.02f);
	ImGui::DragFloat("Birth Size Variation", &particle.BirthSizeVariation, 0.02f);
	ImGui::DragFloat("Death Size", &particle.DeathSize, 0.02f);
	ImGui::DragFloat("Death Size Variation", &particle.DeathSizeVariation, 0.02f);

	ImGui::DragFloat("Rotation Speed", &particle.RotationSpeed);
	ImGui::DragFloat("Rotation Variation", &particle.RotationVariation);
	ImGui::DragFloat("Rotation Speed Variation", &particle.RotationSpeedVariation);

	ImGui::DragFloat("Life Span", &particle.LifeSpan);
	ImGui::DragFloat("Life Span Variation", &particle.LifeSpanVariation);

	ImGui::DragInt("Emission Rate", &s_EmissionRate, 1, 0, 100);
	ImGui::SliderFloat("Speed", &s_Speed, 0.1f, 5.0f);

	if (ImGui::Button("Save"))
	{
		auto filename = SaveFile();
		WriteParticleDataText(filename, particle);
	}

	if (ImGui::Button("Load"))
	{
		std::wstring string = OpenFile();
		if (!string.empty())
			particle = ReadParticleDataText(string);
	}


	ImGui::End();
#endif

	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	static bool p_open = true;
	ImGui::Begin("DockSpace Demo", &p_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		/*
		if (ImGui::BeginMenu("Docking"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

			if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0))     dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
			ImGui::Separator();
			ImGui::EndMenu();
		}
		*/

		ImGui::EndMenuBar();
	}


	if (m_OnImGuiDrawCallback)
		m_OnImGuiDrawCallback();

	DrawImGuiStatsPanel();

	DrawViewport();

	//ImGui::ShowDemoWindow();
	ImGui::End();
}

bool Application::IsKeyPressed(int keycode)
{
	return s_ActiveApplication->m_Window->IsKeyPressed(keycode);
}

std::wstring Application::OpenFile()
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_Window->GetWin32Window();
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = L"Particle Files (*.particle)\0*.particle\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}

std::wstring Application::SaveFile()
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_Window->GetWin32Window();
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = L"Particle Files (*.particle)\0*.particle\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}

