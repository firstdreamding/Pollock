#include "Memory.h"

#include <cstdlib>
#include <memory>
#include <iostream>

#include <vector>

static size_t s_MemoryUsage = 0;

void* operator new(size_t size)
{
	if (size == 0)
		return nullptr;

	if (size > 1024)
	{
		std::cout << "Allocated " << size << " bytes\n";
	}

	void* block = malloc(size + sizeof(size_t));
	if (!block)
		return block;

	*(size_t*)block = size;
	s_MemoryUsage += size;
	return (size_t*)block + 1;
}

void* operator new(size_t size, const char* file, int line)
{
	std::cout << "Allocating " << size << " bytes at " << file << ":" << line << std::endl;
	return operator new(size);
}

void* operator new[](size_t size)
{
	if (size == 0)
		return nullptr;

	if (size > 1024)
	{
		std::cout << "Allocated[] " << size << " bytes\n";
	}

	void* block = malloc(size + sizeof(size_t));
	if (!block)
		return block;

	*(size_t*)block = size;
	s_MemoryUsage += size;
	return (size_t*)block + 1;
}

void* operator new[](size_t size, const char* file, int line)
{
	std::cout << "Allocating[] " << size << " bytes at " << file << ":" << line << std::endl;
	return operator new[](size);
}

void operator delete(void* memory)
{
	if (!memory)
		return;

	void* block = (size_t*)memory - 1;
	size_t size = *(size_t*)block;
	if (size > 1024)
	{
		std::cout << "Freeing " << size << " bytes\n";
	}

	free(block);
	s_MemoryUsage -= size;
}

void operator delete[](void* memory)
{
	if (!memory)
		return;

	void* block = (size_t*)memory - 1;
	size_t size = *(size_t*)block;
	if (size > 1024)
	{
		std::cout << "Freeing[] " << size << " bytes\n";
	}

	free(block);
	s_MemoryUsage -= size;
}

namespace Pollock {

	size_t GetMemoryUsage()
	{
		return s_MemoryUsage;
	}

}