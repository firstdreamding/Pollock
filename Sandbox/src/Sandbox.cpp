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

static ParticleEditor particleEditor;
static Ref<TextureEditor> textureEditor;

static Ref<Texture2D> s_MegaManTexture;
static Ref<SubTexture2D> s_MegaManSpriteSheet;

static int s_Frame = 0;
static int s_AnimationFPS = 5;
static float s_AnimationTimer = 1.0f / (float)s_AnimationFPS;
static float s_Timer = s_AnimationTimer;

static void OnUpdate(float ts)
{
	particleEditor.OnUpdate(ts);

	auto animationTexture = textureEditor->GetAnimationTexture();
	auto texture = textureEditor->GetCurrentTexture();
	if (animationTexture && texture)
	{
		s_Timer -= ts;
		if (s_Timer <= 0.0f)
		{
			s_Timer = s_AnimationTimer;
			s_Frame = (s_Frame + 1);
		}

		s_Frame %= (animationTexture->GetHorizontalSpriteCount() * animationTexture->GetVerticalSpriteCount());
	
		int xc = s_Frame % animationTexture->GetHorizontalSpriteCount();
		int yc = s_Frame / animationTexture->GetHorizontalSpriteCount();

		Renderer::Begin();
		Renderer::DrawTexturedQuad({ 2.0f, 0.0f }, { 1.0f, 1.0f }, texture.get(),
			animationTexture->GetTextureCoords(xc, yc));
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
		if (ImGui::SliderInt("FPS", &s_AnimationFPS, 0, 60))
		{
			s_AnimationTimer = 1.0f / (float)s_AnimationFPS;
		}
		ImGui::End();
	});

	application.SetOnUpdateCallback(OnUpdate);

	application.Run();
}

