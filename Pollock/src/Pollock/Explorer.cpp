#include "Explorer.h"

#include <iostream>
#include <imgui.h>

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
		if (std::filesystem::is_directory(entryPath))
		{
			std::cout << entryPath.filename().string() << "/" << std::endl;
			results.push_back({ entryPath.filename().string(), true });

		}
		else
		{
			std::cout << entryPath.filename().string() << std::endl;
			results.push_back({ entryPath.filename().string(), false });
		}
	}

	return results;
}

Explorer::Explorer(const std::string& rootDir)
{
	m_RootDir = std::filesystem::canonical(rootDir);
	m_CurrentDirectory = m_RootDir;
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
	}

	for (const auto& file : files)
	{
		std::string name = file.Path.string();

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
		}
	}


	ImGui::End();
}
