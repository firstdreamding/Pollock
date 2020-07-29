#pragma once

#include "ParticleSystem.h"

class ParticleSerializer
{
public:
	void Serialize(const std::string& filepath, const std::vector<ParticleInstance>& instances);
	void Deserialize(const std::string& filepath, std::vector<ParticleInstance>& instances);
};