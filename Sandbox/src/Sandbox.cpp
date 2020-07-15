#include <Pollock/Log.h>
#include <Pollock/Window.h>

#include <iostream>

#include <Pollock/Renderer.h>

#include <Pollock/Camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "Pollock/ParticleSystem.h"
#include "Pollock/Random.h"

#include "imgui.h"

#include <fstream>

#include "ParticleEditor.h"

#include "yaml-cpp/yaml.h"

class StackAllocator
{
public:
	StackAllocator(uint32_t capacity)
		: m_Capacity(capacity)
	{
		m_Memory = (uint8_t*)malloc(capacity);
		m_MemoryPtr = (uint8_t*)m_Memory;
		memset(m_Memory, 0xab, capacity);
	}

	~StackAllocator()
	{
		free(m_Memory);
	}

	void* Allocate(uint32_t size)
	{
		if (GetTotalAllocSize() + size > m_Capacity)
		{
			std::cout << "Could not allocate " << size << " bytes!\n";
			return nullptr;
		}

		uint8_t* memory = m_MemoryPtr;
		m_MemoryPtr += size;
		*(uint32_t*)m_MemoryPtr = size; // 4
		m_MemoryPtr += sizeof(uint32_t);

		std::cout << "m_MemoryPtr is now at position " << (m_MemoryPtr - m_Memory) << std::endl;

		return memory;
	}

	template<typename T>
	T* Allocate()
	{
		return (T*)Allocate(sizeof(T));
	}

	void Free(uint32_t size)
	{
		m_MemoryPtr -= size;
#ifdef _DEBUG
		memset(m_MemoryPtr, 0xdd, size);
#endif
	}

	template<typename T>
	void Free()
	{
		Free(sizeof(T));
	}

	void Pop()
	{
		if (m_Memory == m_MemoryPtr)
			return;

		uint32_t size = *(uint32_t*)(m_MemoryPtr - sizeof(uint32_t));
		m_MemoryPtr -= sizeof(uint32_t) + size;

		std::cout << "m_MemoryPtr is now at position " << (m_MemoryPtr - m_Memory) << std::endl;
	}

	uint32_t GetTotalAllocSize() { return m_MemoryPtr - m_Memory; }
private:
	uint8_t* m_Memory;
	uint8_t* m_MemoryPtr;
	uint32_t m_Capacity;
};

static Window* window;

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

static void SetCameraProjection(Camera& camera, uint32_t width, uint32_t height, float zoom)
{
	float aspectRatio = (float)width / (float)height; // 1.78

	float w = width / 1280.0f; // 1.0f
	float h = height / 720.0f; // 1.0f
	float multiplier = zoom;

	glm::mat4 projection = glm::ortho(-aspectRatio * multiplier, aspectRatio * multiplier, -1.0f * multiplier, 1.0f * multiplier);
	camera.SetProjection(projection);
}

static void MoveCamera(Camera& camera, glm::vec2& position, Window* window, float timestep)
{
	float speed = 20.0f * timestep;

	if (window->IsKeyPressed(PL_KEY_W))
		position.y += speed;
	else if (window->IsKeyPressed(PL_KEY_S))
		position.y -= speed;

	if (window->IsKeyPressed(PL_KEY_A))
		position.x -= speed;
	else if (window->IsKeyPressed(PL_KEY_D))
		position.x += speed;

	camera.SetTranslation(position);
}

static float s_Speed = 1.0f;
static ParticleProperties particle;

static void OncePerSecond(int& counter)
{
}

static std::wstring OpenFile()
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = window->GetWin32Window();
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = L"Particle Files (*.particle)\0*.particle\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
}

static std::wstring SaveFile()
{
	TCHAR fileString[256] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = window->GetWin32Window();
	ofn.lpstrFile = fileString;
	ofn.nMaxFile = sizeof(fileString);
	ofn.lpstrFilter = L"Particle Files (*.particle)\0*.particle\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return {};
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

static void WriteParticleDataText(const std::wstring& filepath, const ParticleProperties& particle)
{
	YAML::Emitter out;
	out << YAML::Comment("Particle");
	out << YAML::BeginMap;
	out << YAML::Key << "position" << YAML::Value << particle.Position;
	out << YAML::Key << "velocity" << YAML::Value << particle.Velocity;
	out << YAML::Key << "velocityVariation" << YAML::Value << particle.VelocityVariation;
	out << YAML::Key << "birthColor" << YAML::Value << particle.BirthColor;
	out << YAML::Key << "deathColor" << YAML::Value << particle.DeathColor;
	out << YAML::Key << "birthSize" << YAML::Value << particle.BirthSize;
	out << YAML::Key << "birthSizeVariation" << YAML::Value << particle.BirthSizeVariation;
	out << YAML::Key << "deathSize" << YAML::Value << particle.DeathSize;
	out << YAML::Key << "deathSizeVariation" << YAML::Value << particle.DeathSizeVariation;
	out << YAML::Key << "rotationSpeed" << YAML::Value << particle.RotationSpeed;
	out << YAML::Key << "rotationVariation" << YAML::Value << particle.RotationVariation;
	out << YAML::Key << "rotationSpeedVariation" << YAML::Value << particle.RotationSpeedVariation;
	out << YAML::Key << "lifeSpan" << YAML::Value << particle.LifeSpan;
	out << YAML::Key << "lifeSpanVariation" << YAML::Value << particle.LifeSpanVariation;
	out << YAML::EndMap;

	std::ofstream stream(filepath);
	stream << out.c_str();
	stream.close();
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
	if (data["velocity"])
		result.Velocity = data["velocity"].as<glm::vec2>();
	if (data["velocityVariation"])
		result.VelocityVariation = data["velocityVariation"].as<glm::vec2>();
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

static int s_EmissionRate = 5;

ParticleEditor s_ParticleEditor;

struct Stats
{
	float FPS = 0.0f;
	int Draws = 0;
};

static Stats s_Stats;

static void DrawImGuiStatsPanel()
{
	ImGui::Begin("Stats");
	ImGui::Text("FPS: %.2f", s_Stats.FPS);
	ImGui::Text("Draws: %d", s_Stats.Draws);
	ImGui::End();
}

static void OnImGuiRender()
{
#if 0
	ImGui::Begin("Properties");
	ImGui::DragFloat2("Position", glm::value_ptr(particle.Position), 0.05f);

	ImGui::DragFloat2("Velocity", glm::value_ptr(particle.Velocity), 0.05f);
	ImGui::DragFloat2("Velocity Variation", glm::value_ptr(particle.VelocityVariation), 0.05f);

	ImGui::ColorEdit4("Birth Color", glm::value_ptr(particle.BirthColor));
	ImGui::ColorEdit4("Death Color", glm::value_ptr(particle.DeathColor));

	ImGui::DragFloat("Birth Size", &particle.BirthSize, 0.02f);
	ImGui::DragFloat("Birth Size Variation", &particle.BirthSizeVariation, 0.02f);
	ImGui::DragFloat("Death Size", &particle.DeathSize, 0.02f);
	ImGui::DragFloat("Death Size Variation", &particle.DeathSizeVariation, 0.02f);

	ImGui::DragFloat("Rotation Speed", &particle.RotationSpeed);
	ImGui::DragFloat("Rotation Variation", &particle.RotationVariation);
	ImGui::DragFloat("Rotation Speed Variation", &particle.RotationSpeedVariation);

	ImGui::DragFloat("Life Span", &particle.LifeSpan);
	ImGui::DragFloat("Life Span Variation", &particle.LifeSpanVariation);

	ImGui::DragInt("Emission Rate", &s_EmissionRate, 1, 0, 100);
	ImGui::SliderFloat("Speed", &s_Speed, 0.1f, 5.0f);

	if (ImGui::Button("Save"))
	{
		auto filename = SaveFile();
		WriteParticleDataText(filename, particle);
	}

	if (ImGui::Button("Load"))
	{
		std::wstring string = OpenFile();
		if (!string.empty())
			particle = ReadParticleDataText(string);
	}


	ImGui::End();
#endif

	s_ParticleEditor.OnImGuiDraw();

	DrawImGuiStatsPanel();
}

int main()
{
	// Goals:
	//	- Speed
	//	- Functionality (good API)
	//	- Utility
	//	- Graphics
	//  - Ring Buffer

	// Tests:.
	//	- Test memory allocation speeds

	window = new Window("Pollock", 1280, 720);
	Camera camera(-1.6f, 1.6f, -1.0f, 1.0f);
	float cameraZoom = 1.0f;
	uint32_t cameraWidth = 1280, cameraHeight = 720;
	SetCameraProjection(camera, cameraWidth, cameraHeight, cameraZoom);
	Renderer::Init();
	Random::Init();

	window->SetResizeCallback([&](uint32_t width, uint32_t height)
	{
		cameraWidth = width;
		cameraHeight = height;
		SetCameraProjection(camera, width, height, cameraZoom);
	});

	window->SetMouseScrollCallback([&](float x, float y)
	{
		cameraZoom -= y * 0.5f;
		cameraZoom = glm::max(0.5f, cameraZoom);
		SetCameraProjection(camera, cameraWidth, cameraHeight, cameraZoom);
	});

	window->SetOnImGuiRenderCallback(OnImGuiRender);

	glm::vec2 cameraPosition = { 0.0f, 0.0f };

	auto lastTime = std::chrono::high_resolution_clock::now();

	float timerValue = 1.0f;
	float timer = timerValue;
	int counter = 0;
	
	ParticleSystem particleSystem;
	particle.Position = { 0.0f, 0.0f };
	particle.Velocity = { 0.0f, 4.0f };
	particle.VelocityVariation = { 4.0f, 2.0f };
	particle.BirthColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	particle.DeathColor = { 1.0f, 1.0f, 0.0f, 0.0f };
	particle.BirthSize = 0.2f;
	particle.BirthSizeVariation = 0.1f;
	particle.DeathSize = 0.02f;
	particle.DeathSizeVariation = 0.04f;
	particle.RotationSpeed = 360.0f;
	particle.RotationVariation = 360.0f;
	particle.RotationSpeedVariation = 100.0f;
	particle.LifeSpan = 1.0f;
	particle.LifeSpanVariation = 1.0f;

	ParticleProperties p = particle;
	p.Position = { -2.0f, 0.0f };

#if 0
 	std::ofstream stream("Particle.particle");
	stream << "Particle" << std::endl;
 	stream << "Position=" << particle.Position.x << "," << particle.Position.y << std::endl;
	stream << "BirthSize=" << particle.BirthSize << std::endl;
	stream.close();

	std::string data;
	std::ifstream istream("filename.txt");
	istream >> data;
	istream.close();

	std::cout << "String = " << data << std::endl;

	float data[2];
	file = fopen("Particle.particle", "rb");
	fread(data, 8, 1, file);
	fclose(file);

	std::cout << "Data is " << data[0] << ", " << data[1] << std::endl;
#endif
	

	while (!window->IsClosed())
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto difference = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastTime);
		lastTime = now;
		auto seconds = difference.count() * 0.001f * 0.001f * 0.001f; // 0.033

		timer -= seconds;
		
		MoveCamera(camera, cameraPosition, window, seconds);

		// Rendering
		Renderer::Clear();
		Renderer::SetCamera(camera);
		uint32_t quadCount = 0;

		s_ParticleEditor.OnUpdate(seconds);
		// particleSystem.OnUpdate(seconds * s_Speed);

		/*for (int i = 0; i < s_EmissionRate; i++)
			particleSystem.Emit(particle);

		for (int i = 0; i < s_EmissionRate; i++)
			particleSystem.Emit(p);*/

		s_Stats.Draws = quadCount;

		if (timer <= 0.0f)
		{
			timer = timerValue;
			OncePerSecond(counter);
			s_Stats.FPS = (1.0f / seconds);
		}
		//Renderer::DrawQuad({ -0.5f,  0.5f }, { 0.8f, 0.2f, 0.0f });
		//Renderer::DrawQuad({  0.5f,  0.5f }, { 0.2f, 0.2f, 0.8f });
		//Renderer::DrawQuad({ -0.5f, -0.5f }, { 0.8f, 0.2f, 0.8f });

		window->Update();
	}
}

