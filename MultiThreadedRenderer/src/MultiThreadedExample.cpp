#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>

#include <Windows.h>

#include "../../Pollock/src/Pollock/Timer.h"

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
		if (s_AppThreadFrame - s_RenderThreadFrame > 1)
		{
			WaitForSingleObject(s_UpdateSignal, INFINITE);
		}
		ResetEvent(s_UpdateSignal);

		// PrintFrame("[AppThread] Started frame ", s_AppThreadFrame);
		{
			// Work here
			Timer timer;
			uint32_t frame = s_AppThreadFrame;
			Renderer_Submit([frame]()
			{
				std::cout << "App thread has submitted this work for the render thread\n";
				std::cout << "Frame = " << frame << "\n";
				if (frame % 2 == 0)
					std::cout << "I'm on an even frame!\n";
			});

			//std::this_thread::sleep_for(1ms);
			Log("AppThread took ", timer.ElapsedMillis());
		}
		//PrintFrame("[AppThread] Finished frame ", s_AppThreadFrame);
		s_AppThreadFrame++;
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
	Timer timer;
	auto& queue = s_RenderCommandQueue[GetRenderQueueIndex()];
	for (auto& func : queue)
		func();

	queue.clear();
	Log("ExecuteRenderCommandQueue took ", timer.ElapsedMillis());
}

static void RenderThread()
{
	s_RenderThreadFrame = 0;
	while (true)
	{
		WaitForSingleObject(s_RenderSignal, INFINITE);
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

int main()
{
	s_RenderSignal = CreateEvent(NULL, FALSE, FALSE, "RenderSignal");
	s_UpdateSignal = CreateEvent(NULL, TRUE, FALSE, "UpdateSignal");

	std::thread renderThread(RenderThread);
	AppThread();

	renderThread.join();
}