#include <Pollock/Log.h>
#include <Pollock/Window.h>

#include <iostream>

#include "Pollock/ParticleSystem.h"
#include "Pollock/Application.h"

#include "imgui.h"

#include <fstream>

#include "ParticleEditor.h"

static ParticleEditor particleEditor;

static void OnUpdate(float ts)
{
	particleEditor.OnUpdate(ts);
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

	Camera& camera = application.GetCamera();
	application.SetPostViewportDrawCallback([&](ImVec2 viewportSize)
	{
		particleEditor.DrawGizmo(camera, viewportSize);
	});

	application.SetImGuiDrawCallback([&]()
	{
		particleEditor.OnImGuiDraw();
	});

	application.SetOnUpdateCallback(OnUpdate);

	application.Run();
}

