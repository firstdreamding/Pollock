#pragma once

#include <string>

class Project
{
public:
	Project(const std::string& path);
private:
	std::string m_Path, m_AbsolutePath;
};