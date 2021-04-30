#include <iostream>
#include <thread>
#include <chrono>

#include <Windows.h>

using namespace std::chrono_literals;

static HANDLE s_RenderSignal;
static HANDLE s_UpdateSignal;

std::atomic<uint32_t> s_AppThreadFrame = 0;
std::atomic<uint32_t> s_RenderThreadFrame = 0;

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

		std::cout << "[AppThread] Started frame " << s_AppThreadFrame << std::endl;
		{
			std::this_thread::sleep_for(4ms);
		}
		std::cout << "[AppThread] Finished frame " << s_AppThreadFrame << std::endl;
		s_AppThreadFrame++;
		SetEvent(s_RenderSignal);
	}
}

static void RenderThread()
{
	s_RenderThreadFrame = 0;
	while (true)
	{
		WaitForSingleObject(s_RenderSignal, INFINITE);
		std::cout << "                                        [RenderThread] Started frame " << s_RenderThreadFrame << std::endl;
		{
			std::this_thread::sleep_for(4ms);
		}
		std::cout << "                                        [RenderThread] Finished frame " << s_RenderThreadFrame << std::endl;
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