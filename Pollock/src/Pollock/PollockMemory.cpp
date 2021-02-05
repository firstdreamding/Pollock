#include "Memory.h"

#include <malloc.h>
#include <iostream>

void* operator new(size_t size)
{
	if (size > 1024)
	{
		__debugbreak();
		std::cout << "Allocated " << size << " bytes\n";
	}
	return malloc(size);
}
