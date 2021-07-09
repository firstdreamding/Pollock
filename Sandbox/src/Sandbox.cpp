#include <Pollock/Log.h>
#include <Pollock/Window.h>

#include <iostream>

#include "Pollock/ParticleSystem.h"
#include "Pollock/Application.h"

#include "imgui.h"

#include <fstream>

#include "ParticleEditor.h"
#include "TextureEditor.h"
#include "Pollock/Renderer.h"

#include "Pollock/PollockMemory.h"

#include "Pollock/AnimationPlayer.h"
#include "ApplicationSettings.h"

#include "Pollock/Project.h"
#include "Pollock/Explorer.h"

static ParticleEditor particleEditor;
static std::shared_ptr<TextureEditor> textureEditor;

static Ref<Texture2D> s_MegaManTexture;

static std::shared_ptr<AnimationPlayer> s_AnimationPlayer;

static void OnUpdate(float ts)
{
	particleEditor.OnUpdate(ts);

	s_AnimationPlayer = textureEditor->GetAnimationPlayer();

	if (s_AnimationPlayer)
	{
		s_AnimationPlayer->OnUpdate(ts);

		Renderer::Begin();
		Renderer::DrawTexturedQuad({ 2.0f, 0.0f }, { 1.0f, 1.0f }, s_AnimationPlayer->GetTexture().Raw(),
			s_AnimationPlayer->GetTextureCoords());
		Renderer::End();
	}

	Renderer::ProcessResources();
}

static void Print()
{
	std::cout << "Hello World!\n";
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

	Application application("Pollock", 1280, 720);
	ApplicationSettings appSettings(application.m_ApplicationProp);

	Project project("MyProject");

	particleEditor.LoadFile(L"Smoke.particle");

	textureEditor = std::make_shared<TextureEditor>();

	char* block = new char[256];

	Explorer* e = new (block) Explorer("");
	Explorer* e1 = new (block + sizeof(Explorer)) Explorer("");


	Camera& camera = application.GetCamera();
	application.SetPostViewportDrawCallback([&](ImVec2 viewportSize)
	{
		particleEditor.DrawGizmo(camera, viewportSize);
	});

	Explorer explorer("MyProject");

	application.SetImGuiDrawCallback([&]()
	{
		particleEditor.OnImGuiDraw();
		textureEditor->OnImGuiRender();
		explorer.OnImGuiRender();

		ImGui::Begin("Memory Stats");
		ImGui::Text("Memory Usage: %d bytes", (int)Pollock::GetMemoryUsage());
		ImGui::End();

		ImGui::Begin("Animation");
		if (s_AnimationPlayer)
		{
			float fps = s_AnimationPlayer->GetFrameRate();
			if (ImGui::SliderFloat("FPS", &fps, 0, 60))
			{
				s_AnimationPlayer->SetFrameRate(fps);
			}

			if (ImGui::Button("Assign animation to particles"))
			{
				particleEditor.SetAnimation(s_AnimationPlayer);
			}
		}
		ImGui::End();
		appSettings.OnImGuiDraw();
	});

	application.SetOnUpdateCallback(OnUpdate);

	application.Run();
}

#include <Windows.h>

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	return main();
}