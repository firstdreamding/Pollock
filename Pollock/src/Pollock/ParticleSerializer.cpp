#include "ParticleSerializer.h"

#include <glm/glm.hpp>
#include "yaml-cpp/yaml.h"

#include <fstream>

YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}


YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

static void WriteParticleDataBinary(const std::wstring& filepath, const ParticleProperties& particle)
{
	FILE* file = _wfopen(filepath.c_str(), L"wb");
	fwrite(&particle, sizeof(ParticleProperties), 1, file);
	fclose(file);
}

static ParticleProperties ReadParticleData(const std::wstring& filepath)
{
	FILE* file = _wfopen(filepath.c_str(), L"rb");
	ParticleProperties result;
	fread(&result, sizeof(ParticleProperties), 1, file);
	fclose(file);
	return result;
}


static ParticleProperties ReadParticleDataText(const std::wstring& filepath)
{
	std::ifstream stream(filepath);
	std::stringstream strStream;
	strStream << stream.rdbuf();

	YAML::Node data = YAML::Load(strStream.str());

	ParticleProperties result;

	if (data["position"])
		result.Position = data["position"].as<glm::vec2>();
	if (data["emissionAngle"])
		result.EmissionAngle = data["emissionAngle"].as<float>();
	if (data["emissionForce"])
		result.EmissionForce = data["emissionForce"].as<float>();
	if (data["emissionAngleVariation"])
		result.EmissionAngleVariation = data["emissionAngleVariation"].as<float>();
	if (data["emissionForceVariation"])
		result.EmissionForceVariation = data["emissionForceVariation"].as<float>();
	if (data["birthColor"])
		result.BirthColor = data["birthColor"].as<glm::vec4>();
	if (data["deathColor"])
		result.DeathColor = data["deathColor"].as<glm::vec4>();
	if (data["birthSize"])
		result.BirthSize = data["birthSize"].as<float>();
	if (data["birthSizeVariation"])
		result.BirthSizeVariation = data["birthSizeVariation"].as<float>();
	if (data["deathSize"])
		result.DeathSize = data["deathSize"].as<float>();
	if (data["deathSizeVariation"])
		result.DeathSizeVariation = data["deathSizeVariation"].as<float>();
	if (data["rotationSpeed"])
		result.RotationSpeed = data["rotationSpeed"].as<float>();
	if (data["rotationVariation"])
		result.RotationVariation = data["rotationVariation"].as<float>();
	if (data["rotationSpeedVariation"])
		result.RotationSpeedVariation = data["rotationSpeedVariation"].as<float>();
	if (data["lifeSpan"])
		result.LifeSpan = data["lifeSpan"].as<float>();
	if (data["lifeSpanVariation"])
		result.LifeSpanVariation = data["lifeSpanVariation"].as<float>();

	return result;
}


static void WriteParticle(YAML::Emitter& out, const ParticleInstance& particle)
{
	auto& props = particle.Properties;

	out << YAML::BeginMap; // System
	out << YAML::Key << "Name" << YAML::Value << particle.System->Name;
	out << YAML::Key << "Particle" << YAML::Value;
	out << YAML::BeginMap; // Particle
	out << YAML::Key << "count" << YAML::Value << props->EmissionCount;
	out << YAML::Key << "position" << YAML::Value << props->Position;
	out << YAML::Key << "emissionAngle" << YAML::Value << props->EmissionAngle;
	out << YAML::Key << "emissionForce" << YAML::Value << props->EmissionForce;
	out << YAML::Key << "emissionAngleVariation" << YAML::Value << props->EmissionAngleVariation;
	out << YAML::Key << "emissionForceVariation" << YAML::Value << props->EmissionForceVariation;
	out << YAML::Key << "birthColor" << YAML::Value << props->BirthColor;
	out << YAML::Key << "deathColor" << YAML::Value << props->DeathColor;
	out << YAML::Key << "birthSize" << YAML::Value << props->BirthSize;
	out << YAML::Key << "birthSizeVariation" << YAML::Value << props->BirthSizeVariation;
	out << YAML::Key << "deathSize" << YAML::Value << props->DeathSize;
	out << YAML::Key << "deathSizeVariation" << YAML::Value << props->DeathSizeVariation;
	out << YAML::Key << "rotationSpeed" << YAML::Value << props->RotationSpeed;
	out << YAML::Key << "rotationVariation" << YAML::Value << props->RotationVariation;
	out << YAML::Key << "rotationSpeedVariation" << YAML::Value << props->RotationSpeedVariation;
	out << YAML::Key << "lifeSpan" << YAML::Value << props->LifeSpan;
	out << YAML::Key << "lifeSpanVariation" << YAML::Value << props->LifeSpanVariation;
	out << YAML::EndMap; // Particle
	out << YAML::EndMap; // System
}

static std::vector<ParticleInstance> ReadParticle(const std::wstring& filepath)
{
	std::ifstream stream(filepath);
	std::stringstream strStream;
	strStream << stream.rdbuf();

	YAML::Node dataPS = YAML::Load(strStream.str());

	std::vector<ParticleInstance> result;

	for (std::size_t i = 0; i < dataPS.size(); i++) {
		ParticleProperties instanceParticle;
		ParticleSystem instanceSystem;
		//System
		if (dataPS[i]["Name"]) {
			//turn string to char array
			std::string tempS = dataPS[i]["Name"].as<std::string>();
			for (int j = 0; j < sizeof(instanceSystem.Name) && j < tempS.length(); j++) {
				instanceSystem.Name[j] = tempS[j];
			}
		}
		
		//Particle
		YAML::Node data = dataPS[i]["Particle"];
		if (data["count"])
			instanceParticle.EmissionCount = data["count"].as<uint32_t>();
		if (data["position"])
			instanceParticle.Position = data["position"].as<glm::vec2>();
		if (data["birthColor"])
			instanceParticle.BirthColor = data["birthColor"].as<glm::vec4>();
		if (data["deathColor"])
			instanceParticle.DeathColor = data["deathColor"].as<glm::vec4>();
		if (data["birthSize"])
			instanceParticle.BirthSize = data["birthSize"].as<float>();
		if (data["birthSizeVariation"])
			instanceParticle.BirthSizeVariation = data["birthSizeVariation"].as<float>();
		if (data["deathSize"])
			instanceParticle.DeathSize = data["deathSize"].as<float>();
		if (data["deathSizeVariation"])
			instanceParticle.DeathSizeVariation = data["deathSizeVariation"].as<float>();
		if (data["rotationSpeed"])
			instanceParticle.RotationSpeed = data["rotationSpeed"].as<float>();
		if (data["rotationVariation"])
			instanceParticle.RotationVariation = data["rotationVariation"].as<float>();
		if (data["rotationSpeedVariation"])
			instanceParticle.RotationSpeedVariation = data["rotationSpeedVariation"].as<float>();
		if (data["lifeSpan"])
			instanceParticle.LifeSpan = data["lifeSpan"].as<float>();
		if (data["lifeSpanVariation"])
			instanceParticle.LifeSpanVariation = data["lifeSpanVariation"].as<float>();
		if (data["emissionAngle"])
			instanceParticle.EmissionAngle = data["emissionAngle"].as<float>();
		if (data["emissionForce"])
			instanceParticle.EmissionForce = data["emissionForce"].as<float>();
		if (data["emissionAngleVariation"])
			instanceParticle.EmissionAngleVariation = data["emissionAngleVariation"].as<float>();
		if (data["emissionForceVariation"])
			instanceParticle.EmissionForceVariation = data["emissionForceVariation"].as<float>();

		//ParticleInstance
		result.push_back({ std::make_shared<ParticleSystem>(instanceSystem), std::make_shared<ParticleProperties>(instanceParticle) });
	}

	return result;
}

void ParticleSerializer::Serialize(const std::wstring& filepath, const std::vector<ParticleInstance>& instances)
{
	YAML::Emitter out;
	out << YAML::Comment("Particle Editor");

	out << YAML::BeginSeq;
	for (const ParticleInstance& pi : instances)
	{
		WriteParticle(out, pi);
	}
	out << YAML::EndSeq;

	std::ofstream stream(filepath);
	stream << out.c_str();
	stream.close();
}

void ParticleSerializer::Deserialize(const std::wstring& filepath, std::vector<ParticleInstance>& instances)
{
	instances = ReadParticle(filepath);
}
