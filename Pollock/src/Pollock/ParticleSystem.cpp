#include "ParticleSystem.h"

#include <glm/gtx/compatibility.hpp>

#include "Renderer.h"
#include "Random.h"

ParticleSystem::ParticleSystem(uint32_t maxParticles)
{
	m_ParticlePool.resize(maxParticles);
}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::Emit(const ParticleProperties& particleProps)
{
	for (uint32_t i = 0; i < particleProps.EmissionCount; i++)
		EmitSingle(particleProps);
}

void ParticleSystem::EmitSingle(const ParticleProperties& particleProps)
{
	Particle& particle = m_ParticlePool[m_ParticlePoolIndex];
	particle.Position = particleProps.Position;

	float forceVariation = particleProps.EmissionForceVariation * Random::Float() - particleProps.EmissionForceVariation * 0.5f;
	float velocityVariation = particleProps.EmissionAngleVariation * Random::Float() - particleProps.EmissionAngleVariation * 0.5f;

	//particle.Velocity = particleProps.Velocity + velocityVariation;
	particle.Velocity = { glm::cos(particleProps.EmissionAngle + forceVariation) * (particleProps.EmissionForce + velocityVariation),
		glm::sin(particleProps.EmissionAngle + forceVariation) * (particleProps.EmissionForce + velocityVariation)};
	particle.BirthColor = particleProps.BirthColor;
	particle.DeathColor = particleProps.DeathColor;

	if (particleProps.RotationSpeed != 0.0f)
	{
		float rotationVariation = (particleProps.RotationVariation * Random::Float()) - particleProps.RotationVariation * 0.5f;
		float rotationSpeedVariation = (particleProps.RotationSpeedVariation * Random::Float()) - particleProps.RotationSpeedVariation * 0.5f;

		particle.Rotation = glm::radians(rotationVariation);
		particle.RotationSpeed = glm::radians(particleProps.RotationSpeed) + glm::radians(rotationVariation);
	}

	float birthSizeVariation = (particleProps.BirthSizeVariation * Random::Float()) - particleProps.BirthSizeVariation * 0.5f;
	float deathSizeVariation = (particleProps.DeathSizeVariation * Random::Float()) - particleProps.DeathSizeVariation * 0.5f;
	particle.BirthSize = particleProps.BirthSize + birthSizeVariation;
	particle.DeathSize = particleProps.DeathSize + deathSizeVariation;

	float lifeVariation = (particleProps.LifeSpanVariation * Random::Float()) - particleProps.LifeSpanVariation * 0.5f;
	particle.LifeSpan = particleProps.LifeSpan + lifeVariation;
	particle.LifeRemaining = particle.LifeSpan;
	particle.Active = true;

	m_ParticlePoolIndex = (m_ParticlePoolIndex + 1) % m_ParticlePool.size();
}

void ParticleSystem::OnUpdate(float ts, bool wireframe)
{
	Renderer::SetWireframe(wireframe);
	Renderer::Begin();
	for (size_t i = 0; i < m_ParticlePool.size(); i++)
	{
		Particle& particle = m_ParticlePool[i];
		if (!particle.Active)
			continue;

		particle.LifeRemaining -= ts;
		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
			continue;
		}

		//particle.Velocity.y -= ts * 10.0f;
		particle.Position += particle.Velocity * ts;

		particle.Rotation += particle.RotationSpeed * ts;

		float life = particle.LifeRemaining / particle.LifeSpan;
		glm::vec4 color = glm::lerp(particle.DeathColor, particle.BirthColor, life);
		float size = glm::lerp(particle.DeathSize, particle.BirthSize, life);
		Renderer::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);
	}
	Renderer::End();
	if (wireframe)
		Renderer::SetWireframe(false);
}
