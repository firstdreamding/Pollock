#include "FileDialogs.h"

#include <Windows.h>

std::string FileDialogs::OpenFile(const char* filter)
{
	char fileString[256] = { 0 };

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}

std::string FileDialogs::SaveFile(const char* filter)
{
	char fileString[256] = { 0 };

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}


std::wstring FileDialogs::OpenFileW(const wchar_t* filter)
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}

std::wstring FileDialogs::SaveFileW(const wchar_t* filter)
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}
