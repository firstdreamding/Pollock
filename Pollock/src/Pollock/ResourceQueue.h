#pragma once

#include <queue>
#include <mutex>
#include <unordered_map>

#include "Texture.h"

class ResourceQueue
{
public:
	ResourceQueue();

	void UploadTexture(Image2D* texture);

	void Update();

	Texture2D* GetTexture(const std::string& path) const
	{
		if (m_LoadedTextures.find(path) != m_LoadedTextures.end())
			return m_LoadedTextures.at(path);

		return nullptr;
	}
private:
	std::queue<Image2D*> m_ImageUploadQueue;
	std::mutex m_ImageUploadQueueMutex;

	// TODO: Asset Manager
	std::unordered_map<std::string, Texture2D*> m_LoadedTextures;
};