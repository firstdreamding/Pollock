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

#include "Application.h"
#include "Renderer.h"

#include "PollockMemory.h"

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
	while (Application::Get()->IsRunning())
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
			Image2D* image = pnew Image2D(path);
			Renderer::GetResourceQueue().UploadTexture(image);
			g_LoadedImages[path] = image;
			imageQueue.pop();
		}

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5ms);
	}

	std::cout << "Loading thread shutting down...\n";
}

Explorer::Explorer(const std::string& rootDir)
{
	m_RootDir = std::filesystem::canonical(rootDir);
	m_CurrentDirectory = m_RootDir;
	
	Application::Get()->CreateThread(LoadingThread);
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

	float thumbnailWidth = 128;

	auto[panelWidth, panelHeight] = ImGui::GetContentRegionAvail();


	float padding = 10.0f;
	int columnCount = panelWidth / (thumbnailWidth + padding);
	// int rowCount = ;

	//ImGui::Columns(columnCount);

	int currentColumn = 0;

	for (const auto& file : files)
	{
		std::string name = file.Path.filename().string();

		if (file.IsDirectory)
		{
			//if (ImGui::Button(name.c_str()))
			if (ImGui::Button(name.c_str(), { thumbnailWidth, thumbnailWidth }))
			{
				m_CurrentDirectory /= file.Path.filename();
			}
		}
		else
		{
			
			
			std::string extension = file.Path.extension().string();
			if (extension == ".png" || extension == ".jpg")
			{
				std::string path = file.Path.string();

				LoadThumbnail(path);

				auto& rq = Renderer::GetResourceQueue();
				Texture2D* texture = rq.GetTexture(path);
				if (texture)
					ImGui::Image((ImTextureID)texture->GetRendererID(), { thumbnailWidth, thumbnailWidth });
				else
					ImGui::Button(name.c_str(), { thumbnailWidth, thumbnailWidth });
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
		currentColumn++;
		if (currentColumn >= columnCount)
			currentColumn = 0;
		else
			ImGui::SameLine();
	}

	ImGui::End();
}

static std::unordered_map<std::string, std::shared_ptr<Texture2D>> s_ThumbnailCache;

void Explorer::LoadThumbnail(const std::string& path)
{
	// TODO: ThumbnailCache class
	//if (s_ThumbnailCache.find(path) != s_ThumbnailCache.end())
	//	return;

	// std::lock_guard<std::mutex> queueLock(s_ImageLoadQueueMutex);
	// s_ImageLoadQueue.push(path);
	// g_LoadedImages.emplace(path, nullptr);

}
