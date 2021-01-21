#include "ResourceQueue.h"

#include <iostream>

ResourceQueue::ResourceQueue()
{
}

void ResourceQueue::UploadTexture(Image2D* image)
{
	std::cout << "Queued texture for upload: " << image->GetPath() << std::endl;
	std::lock_guard<std::mutex> queueLock(m_ImageUploadQueueMutex);
	m_ImageUploadQueue.push(image);
}

void ResourceQueue::Update()
{
	Image2D* image = nullptr;
	{
		std::lock_guard<std::mutex> queueLock(m_ImageUploadQueueMutex);
		if (!m_ImageUploadQueue.empty())
		{
			image = m_ImageUploadQueue.front();
			m_ImageUploadQueue.pop();
		}
	}

	if (image)
	{
		std::cout << "ResourceQueue upload: " << image->GetPath() << std::endl;
		m_LoadedTextures[image->GetPath()] = new Texture2D(image);
	}
}