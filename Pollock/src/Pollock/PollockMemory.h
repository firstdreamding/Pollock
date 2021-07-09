#pragma once

namespace Pollock {

	size_t GetMemoryUsage();

}

void* operator new(size_t size, const char* file, int line);
//void* operator new[](size_t size, const char* file, int line);
#define pnew new(__FILE__, __LINE__)
//#define pnew new[](__FILE__, __LINE__)
