#include "Explorer.h"

#include <iostream>
#include <imgui.h>
#include <thread>
#include <chrono>

#include "Texture.h"
#include <queue>

#include <mutex>
#include <scoped_allocator>
#include <unordered_map>

static std::thread s_ImageLoadThread;
static std::queue<std::string> s_ImageLoadQueue;
std::unordered_map<std::string, Image2D*> g_LoadedImages;

static std::mutex s_ImageLoadQueueMutex;

struct FileInfo
{
	std::filesystem::path Path;
	bool IsDirectory;
};

static std::vector<FileInfo> ListInDirectory(std::filesystem::path path)
{
	namespace fs = std::filesystem;

	std::vector<FileInfo> results;

	std::filesystem::directory_iterator di(path);
	for (const auto& entry : di)
	{
		const auto& entryPath = entry.path();
		results.push_back({ entryPath, std::filesystem::is_directory(entryPath) });
	}

	return results;
}

static void LoadingThread()
{
	while (true)
	{
		std::queue<std::string> imageQueue;
		{
			std::lock_guard<std::mutex> queueLock(s_ImageLoadQueueMutex);
			imageQueue = s_ImageLoadQueue;
			s_ImageLoadQueue = std::queue<std::string>();
		}

		while (!imageQueue.empty())
		{
			auto& path = imageQueue.front();
			g_LoadedImages[path] = new Image2D(path);
			imageQueue.pop();
		}

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5ms);
	}
}

Explorer::Explorer(const std::string& rootDir)
{
	m_RootDir = std::filesystem::canonical(rootDir);
	m_CurrentDirectory = m_RootDir;

	s_ImageLoadThread = std::thread(LoadingThread);
}

void Explorer::OnImGuiRender()
{
	ImGui::Begin("Explorer");

	auto files = ListInDirectory(m_CurrentDirectory);

	if (m_CurrentDirectory.has_parent_path())
	{
		if (ImGui::Button("<-"))
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}

		ImGui::SameLine();
	}

	std::string filepath = m_CurrentDirectory.string();
	ImGui::Text("%s", filepath.c_str());

	for (const auto& file : files)
	{
		std::string name = file.Path.filename().string();

		if (file.IsDirectory)
		{
			if (ImGui::Button(name.c_str()))
			{
				m_CurrentDirectory /= file.Path.filename();
			}
		}
		else
		{
			ImGui::Text(name.c_str());

			std::string extension = file.Path.extension().string();
			if (extension == ".png" || extension == ".jpg")
			{
				std::string path = file.Path.string();
				if (g_LoadedImages.find(path) == g_LoadedImages.end())
				{
					std::lock_guard<std::mutex> queueLock(s_ImageLoadQueueMutex);
					s_ImageLoadQueue.push(path);
					g_LoadedImages.emplace(path, nullptr);
				}
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				std::string pathStr = file.Path.string();
				const char* data = pathStr.c_str();
				ImGui::SetDragDropPayload("ASSET_DRAG_DROP", data, pathStr.size());
				ImGui::Text("%s", name.c_str());
				ImGui::EndDragDropSource();
			}
		}
	}

	ImGui::End();
}
