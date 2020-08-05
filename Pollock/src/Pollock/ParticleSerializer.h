#pragma once

#include "ParticleSystem.h"

class ParticleSerializer
{
public:
	void Serialize(const std::wstring& filepath, const std::vector<ParticleInstance>& instances);
	void Deserialize(const std::wstring& filepath, std::vector<ParticleInstance>& instances);
};