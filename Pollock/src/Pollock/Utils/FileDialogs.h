#pragma once

#include <string>

class FileDialogs
{
public:
	static std::string OpenFile(const char* filter);
	static std::string SaveFile(const char* filter);
	static std::wstring OpenFileW(const wchar_t* filter);
	static std::wstring SaveFileW(const wchar_t* filter);
};