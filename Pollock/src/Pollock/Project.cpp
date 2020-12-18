#include "Project.h"

#include <filesystem>
#include <iostream>

struct FileInfo
{
	std::string Path;
	bool IsDirectory;
};

static std::vector<FileInfo> ListInDirectory(std::string path)
{
	namespace fs = std::filesystem;

	std::vector<FileInfo> results;

	std::filesystem::path p(path);
	std::filesystem::directory_iterator di(p);
	for (const auto& entry : di)
	{
		const auto& entryPath = entry.path();
		if (std::filesystem::is_directory(entryPath))
		{
			std::cout << entryPath.filename().string() << "/" << std::endl;
			results.push_back({ entryPath.filename().string(), true});

		}
		else
		{
			std::cout << entryPath.filename().string() << std::endl;
			results.push_back({ entryPath.filename().string(), false });
		}
	}

	return results;
}

Project::Project(const std::string& path)
	: m_Path(path)
{
	std::filesystem::path p(path);
	auto absolutePath  = std::filesystem::canonical(p);
	m_AbsolutePath = absolutePath.string();

	auto files = ListInDirectory(path);
	std::cout << "----------------------\n";
	std::filesystem::path path2(path);
	auto dir = std::find_if(files.begin(), files.end(), [](auto& fi) {return fi.IsDirectory; });
	if (dir != files.end())
	{
		path2 /= dir->Path;
		ListInDirectory(path2.string());
	}
	
}
