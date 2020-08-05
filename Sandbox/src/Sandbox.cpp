#include <Pollock/Log.h>
#include <Pollock/Window.h>

#include <iostream>

#include <Pollock/Renderer.h>
#include <Pollock/Framebuffer.h>

#include <Pollock/Camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "Pollock/ParticleSystem.h"
#include "Pollock/Random.h"

#include "imgui.h"

#include <fstream>

#include "ParticleEditor.h"


class StackAllocator
{
public:
	StackAllocator(uint32_t capacity)
		: m_Capacity(capacity)
	{
		m_Memory = (uint8_t*)malloc(capacity);
		m_MemoryPtr = (uint8_t*)m_Memory;
		memset(m_Memory, 0xab, capacity);
	}

	~StackAllocator()
	{
		free(m_Memory);
	}

	void* Allocate(uint32_t size)
	{
		if (GetTotalAllocSize() + size > m_Capacity)
		{
			std::cout << "Could not allocate " << size << " bytes!\n";
			return nullptr;
		}

		uint8_t* memory = m_MemoryPtr;
		m_MemoryPtr += size;
		*(uint32_t*)m_MemoryPtr = size; // 4
		m_MemoryPtr += sizeof(uint32_t);

		std::cout << "m_MemoryPtr is now at position " << (m_MemoryPtr - m_Memory) << std::endl;

		return memory;
	}

	template<typename T>
	T* Allocate()
	{
		return (T*)Allocate(sizeof(T));
	}

	void Free(uint32_t size)
	{
		m_MemoryPtr -= size;
#ifdef _DEBUG
		memset(m_MemoryPtr, 0xdd, size);
#endif
	}

	template<typename T>
	void Free()
	{
		Free(sizeof(T));
	}

	void Pop()
	{
		if (m_Memory == m_MemoryPtr)
			return;

		uint32_t size = *(uint32_t*)(m_MemoryPtr - sizeof(uint32_t));
		m_MemoryPtr -= sizeof(uint32_t) + size;

		std::cout << "m_MemoryPtr is now at position " << (m_MemoryPtr - m_Memory) << std::endl;
	}

	uint32_t GetTotalAllocSize() { return m_MemoryPtr - m_Memory; }
private:
	uint8_t* m_Memory;
	uint8_t* m_MemoryPtr;
	uint32_t m_Capacity;
};

static Window* window;
static Camera camera(-1.6f, 1.6f, -1.0f, 1.0f);
static float cameraZoom = 1.0f;

static void SetCameraProjection(Camera& camera, uint32_t width, uint32_t height, float zoom)
{
	float aspectRatio = (float)width / (float)height; // 1.78
	float multiplier = zoom;

	glm::mat4 projection = glm::ortho(-aspectRatio * multiplier, aspectRatio * multiplier, -1.0f * multiplier, 1.0f * multiplier);
	camera.SetProjection(projection);
}

static void MoveCamera(Camera& camera, glm::vec2& position, Window* window, float timestep)
{
	float speed = 20.0f * timestep;

	if (window->IsKeyPressed(PL_KEY_W))
		position.y += speed;
	else if (window->IsKeyPressed(PL_KEY_S))
		position.y -= speed;

	if (window->IsKeyPressed(PL_KEY_A))
		position.x -= speed;
	else if (window->IsKeyPressed(PL_KEY_D))
		position.x += speed;

	camera.SetTranslation(position);
}

static float s_Speed = 1.0f;
static ParticleProperties particle;


static std::unique_ptr<Framebuffer> framebuffer;

static void OncePerSecond(int& counter)
{
}

static std::wstring OpenFile()
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = window->GetWin32Window();
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

static std::wstring SaveFile()
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = window->GetWin32Window();
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


static int s_EmissionRate = 5;

ParticleEditor s_ParticleEditor;

struct Stats
{
	float FPS = 0.0f;
	int Draws = 0;
};

static Stats s_Stats;

static void DrawImGuiStatsPanel()
{
	ImGui::Begin("Stats");
	ImGui::Text("FPS: %.2f", s_Stats.FPS);
	ImGui::Text("Draws: %d", s_Stats.Draws);
	ImGui::End();
}

static float s_ViewportWidth = 0.0f, s_ViewportHeight = 0.0f;

std::vector<std::function<void()>> m_PostRenderQueue;

bool s_NeedsResize = false;

static void ResizeIfNeeded(uint32_t width, uint32_t height)
{
	if (s_ViewportWidth == width && s_ViewportHeight == height)
		return;

	s_ViewportWidth = width;
	s_ViewportHeight = height;

	m_PostRenderQueue.push_back([&]()
	{
		framebuffer->Resize(s_ViewportWidth, s_ViewportHeight);
		SetCameraProjection(camera, s_ViewportWidth, s_ViewportHeight, cameraZoom);
	});
}

static void DrawViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::Begin("Viewport");
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	ImGui::Image((void*)framebuffer->GetColorBufferRendererID(), viewportSize, { 0, 1 }, { 1, 0 });
	ResizeIfNeeded(viewportSize.x, viewportSize.y);
	ImGui::End();
	ImGui::PopStyleVar();
}

static void OnImGuiRender()
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

		if (ImGui::BeginMenu("File")) 
		{
			s_ParticleEditor.OnMenuImGuiDraw();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	s_ParticleEditor.OnImGuiDraw();

	DrawImGuiStatsPanel();

	DrawViewport();


	ImGui::End();


}

int main()
{
	// Goals:
	//	- Speed
	//	- Functionality (good API)
	//	- Utility
	//	- Graphics
	//  - Ring Buffer

	// Tests:.
	//	- Test memory allocation speeds

	window = new Window("Pollock", 1280, 720);
	Renderer::Init();
	Random::Init();

	window->SetResizeCallback([&](uint32_t width, uint32_t height)
	{
		// cameraWidth = width;
		// cameraHeight = height;
		// SetCameraProjection(camera, width, height, cameraZoom);
	});

	window->SetMouseScrollCallback([&](float x, float y)
	{
		cameraZoom -= y * 0.5f;
		cameraZoom = glm::max(0.5f, cameraZoom);
		SetCameraProjection(camera, s_ViewportWidth, s_ViewportHeight, cameraZoom);
	});

	window->SetOnImGuiRenderCallback(OnImGuiRender);

	glm::vec2 cameraPosition = { 0.0f, 0.0f };

	auto lastTime = std::chrono::high_resolution_clock::now();

	float timerValue = 1.0f;
	float timer = timerValue;
	int counter = 0;
	
	ParticleSystem particleSystem;
	particle.Position = { 0.0f, 0.0f };
	particle.Velocity = { 0.0f, 4.0f };
	particle.VelocityVariation = { 4.0f, 2.0f };
	particle.BirthColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	particle.DeathColor = { 1.0f, 1.0f, 0.0f, 0.0f };
	particle.BirthSize = 0.2f;
	particle.BirthSizeVariation = 0.1f;
	particle.DeathSize = 0.02f;
	particle.DeathSizeVariation = 0.04f;
	particle.RotationSpeed = 360.0f;
	particle.RotationVariation = 360.0f;
	particle.RotationSpeedVariation = 100.0f;
	particle.LifeSpan = 1.0f;
	particle.LifeSpanVariation = 1.0f;

	ParticleProperties p = particle;
	p.Position = { -2.0f, 0.0f };

#if 0
 	std::ofstream stream("Particle.particle");
	stream << "Particle" << std::endl;
 	stream << "Position=" << particle.Position.x << "," << particle.Position.y << std::endl;
	stream << "BirthSize=" << particle.BirthSize << std::endl;
	stream.close();

	std::string data;
	std::ifstream istream("filename.txt");
	istream >> data;
	istream.close();

	std::cout << "String = " << data << std::endl;

	float data[2];
	file = fopen("Particle.particle", "rb");
	fread(data, 8, 1, file);
	fclose(file);

	std::cout << "Data is " << data[0] << ", " << data[1] << std::endl;
#endif
	
	framebuffer = std::make_unique<Framebuffer>(1280, 720);

	while (!window->IsClosed())
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto difference = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastTime);
		lastTime = now;
		auto seconds = difference.count() * 0.001f * 0.001f * 0.001f; // 0.033

		timer -= seconds;
		
		MoveCamera(camera, cameraPosition, window, seconds);

		// Rendering
		framebuffer->Bind();
		Renderer::Clear();
		Renderer::SetCamera(camera);
		uint32_t quadCount = 0;

		s_ParticleEditor.OnUpdate(seconds);
		// particleSystem.OnUpdate(seconds * s_Speed);

		/*for (int i = 0; i < s_EmissionRate; i++)
			particleSystem.Emit(particle);

		for (int i = 0; i < s_EmissionRate; i++)
			particleSystem.Emit(p);*/

		s_Stats.Draws = quadCount;

		if (timer <= 0.0f)
		{
			timer = timerValue;
			OncePerSecond(counter);
			s_Stats.FPS = (1.0f / seconds);
		}
		//Renderer::DrawQuad({ -0.5f,  0.5f }, { 0.8f, 0.2f, 0.0f });
		//Renderer::DrawQuad({  0.5f,  0.5f }, { 0.2f, 0.2f, 0.8f });
		//Renderer::DrawQuad({ -0.5f, -0.5f }, { 0.8f, 0.2f, 0.8f });

		framebuffer->Unbind();

		window->Update();

		for (auto& func : m_PostRenderQueue)
			func();

		m_PostRenderQueue.clear();
	}
}

