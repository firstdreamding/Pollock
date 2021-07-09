#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>

#include <Windows.h>

#include "../../Pollock/src/Pollock/Timer.h"

#include "Instrumentor.h"

#include "Ref.h"

using namespace std::chrono_literals;

static HANDLE s_RenderSignal;
static HANDLE s_UpdateSignal;

using RenderCommandFn = std::function<void()>;

std::atomic<uint32_t> s_AppThreadFrame = 0;
std::atomic<uint32_t> s_RenderThreadFrame = 0;

std::mutex s_PrintMutex;

constexpr uint32_t s_RenderCommandQueueCount = 2;
static std::vector<RenderCommandFn> s_RenderCommandQueue[s_RenderCommandQueueCount];
static std::atomic<uint32_t> s_RenderCommandQueueSubmissionIndex = 0;

template<typename T>
static void Log(const char* text, T data)
{
	std::scoped_lock lock(s_PrintMutex);
	std::cout << text << data << std::endl;
}

static void Renderer_Submit(const RenderCommandFn& func)
{
	auto& queue = s_RenderCommandQueue[s_RenderCommandQueueSubmissionIndex];
	queue.emplace_back(func);
}

static void SwapQueues()
{
	s_RenderCommandQueueSubmissionIndex = (s_RenderCommandQueueSubmissionIndex + 1) % s_RenderCommandQueueCount;
}

static void AppThread()
{
	s_AppThreadFrame = 0;
	while (true)
	{
		ScopedInstrumentationTimer timer("AppThread-Loop");

		if (s_AppThreadFrame - s_RenderThreadFrame > 1)
		{
			ScopedInstrumentationTimer waitTimer("AppThread-WaitForSingleObject");
			WaitForSingleObject(s_UpdateSignal, INFINITE);
		}
		ResetEvent(s_UpdateSignal);

		// PrintFrame("[AppThread] Started frame ", s_AppThreadFrame);
		{
			// Work here
			ScopedInstrumentationTimer instrumentor("AppThread-Work");
			Timer timer;
			uint32_t frame = s_AppThreadFrame;
			Renderer_Submit([frame]()
			{
				std::cout << "App thread has submitted this work for the render thread\n";
				std::cout << "Frame = " << frame << "\n";
				if (frame % 2 == 0)
					std::cout << "I'm on an even frame!\n";
			});
			std::this_thread::sleep_for(1ms);
			//std::this_thread::sleep_for(1ms);
			Log("AppThread took ", timer.ElapsedMillis());
		}
		//PrintFrame("[AppThread] Finished frame ", s_AppThreadFrame);
		s_AppThreadFrame++;

		if (s_AppThreadFrame == 10)
		{
			Instrumentor::Get().EndSession();
		}
		SwapQueues();
		SetEvent(s_RenderSignal);
	}
}

static uint32_t GetRenderQueueIndex()
{
	return (s_RenderCommandQueueSubmissionIndex + 1) % s_RenderCommandQueueCount;
}

static void ExecuteRenderCommandQueue()
{
	ScopedInstrumentationTimer instrumentor("ExecuteRenderCommandQueue");
	Timer timer;
	auto& queue = s_RenderCommandQueue[GetRenderQueueIndex()];
	for (auto& func : queue)
		func();

	std::this_thread::sleep_for(2ms);
	queue.clear();
	Log("ExecuteRenderCommandQueue took ", timer.ElapsedMillis());
}

static void RenderThread()
{
	s_RenderThreadFrame = 0;
	while (true)
	{
		ScopedInstrumentationTimer timer("RenderThread-Loop");
		{
			ScopedInstrumentationTimer waitTimer("RenderThread-WaitForSingleObject");
			WaitForSingleObject(s_RenderSignal, INFINITE);
		}
		//PrintFrame("                                        [RenderThread] Started frame ", s_RenderThreadFrame);
		{
			// Work here
			ExecuteRenderCommandQueue();
			//std::this_thread::sleep_for(1ms);
		}
		//PrintFrame("                                        [RenderThread] Finished frame ", s_RenderThreadFrame);
		s_RenderThreadFrame++;
		SetEvent(s_UpdateSignal);
	}
}

class Shader : public RefCounted
{
public:
	Shader()
	{
		std::cout << "Created Shader!\n";
	}

	~Shader()
	{
		std::cout << "Destroyed Shader!\n";
	}

	void Func() { std::cout << "Func!\n"; }
};

int main()
{
#if 0
	Instrumentor::Get().BeginSession("MultiThreadedExample");

	s_RenderSignal = CreateEvent(NULL, FALSE, FALSE, "RenderSignal");
	s_UpdateSignal = CreateEvent(NULL, TRUE, FALSE, "UpdateSignal");

	std::thread renderThread(RenderThread);
	AppThread();

	renderThread.join();
#endif

	Ref<Shader> shaderAlive;
	{
		std::cout << "Start Scope\n";
		Ref<Shader> shader = Ref<Shader>::Create();
		shader->Func();
		(*shader).Func();
		shaderAlive = shader;
	}
	std::cout << "End Scope\n";
	system("PAUSE");
}