#pragma once

#include <string>
#include <filesystem>

class Explorer
{
public:
	Explorer(const std::string& rootDir);

	void OnImGuiRender();
private:
	std::filesystem::path m_RootDir;
	std::filesystem::path m_CurrentDirectory;
};
