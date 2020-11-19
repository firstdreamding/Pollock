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

#include "Pollock/AnimationPlayer.h"

static ParticleEditor particleEditor;
static Ref<TextureEditor> textureEditor;

static Ref<Texture2D> s_MegaManTexture;

static Ref<AnimationPlayer> s_AnimationPlayer;

static void OnUpdate(float ts)
{
	particleEditor.OnUpdate(ts);

	s_AnimationPlayer = textureEditor->GetAnimationPlayer();

	if (s_AnimationPlayer)
	{
		s_AnimationPlayer->OnUpdate(ts);

		Renderer::Begin();
		Renderer::DrawTexturedQuad({ 2.0f, 0.0f }, { 1.0f, 1.0f }, s_AnimationPlayer->GetTexture().get(),
			s_AnimationPlayer->GetTextureCoords());
		Renderer::End();
	}
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

	particleEditor.LoadFile(L"Smoke.particle");

	textureEditor = std::make_shared<TextureEditor>();

	Camera& camera = application.GetCamera();
	application.SetPostViewportDrawCallback([&](ImVec2 viewportSize)
	{
		particleEditor.DrawGizmo(camera, viewportSize);
	});

	application.SetImGuiDrawCallback([&]()
	{
		particleEditor.OnImGuiDraw();
		textureEditor->OnImGuiRender();

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
	});

	application.SetOnUpdateCallback(OnUpdate);

	application.Run();
}

