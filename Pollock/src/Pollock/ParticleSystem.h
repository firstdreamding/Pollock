#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

struct ParticleProperties
{
	uint32_t EmissionCount = 1;

	glm::vec2 Position;
	glm::vec4 BirthColor, DeathColor;

	float EmissionAngle = 0.0f, EmissionForce = 0.0f;
	float EmissionAngleVariation = 0.0f, EmissionForceVariation = 0.0f;

	float RotationSpeed = 0.0f;
	float RotationVariation = 0.0f, RotationSpeedVariation = 0.0f;

	float BirthSize, DeathSize, BirthSizeVariation = 0.0f, DeathSizeVariation = 0.0f;
	float LifeSpan, LifeSpanVariation = 0.0f;
};

class ParticleSystem
{
public:
	ParticleSystem(uint32_t maxParticles = 10000);
	~ParticleSystem();

	void Emit(const ParticleProperties& particleProps);
	void EmitSingle(const ParticleProperties& particleProps);
	void OnUpdate(float ts, bool wireframe = false);

	char Name[32] = "New Particle System";

private:
	struct Particle // 73 bytes
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec4 BirthColor, DeathColor;
		
		float Rotation = 0.0f;
		float RotationSpeed = 0.0f;
		float BirthSize, DeathSize;
		float LifeSpan;
		float LifeRemaining;

		bool Active = false;
	};

	std::vector<Particle> m_ParticlePool; // 100,000
	uint32_t m_ParticlePoolIndex = 0;
};

struct ParticleInstance
{
	std::shared_ptr<ParticleSystem> System;
	std::shared_ptr<ParticleProperties> Properties;
};
