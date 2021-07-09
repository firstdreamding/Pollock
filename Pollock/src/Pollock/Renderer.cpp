#include "Renderer.h"

#include <iostream>

#include "glad/glad.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "PollockMemory.h"

#include <array>

struct RendererData
{
	const uint32_t MaxQuads = 100000;
	const uint32_t MaxVertices = MaxQuads * 4;
	const uint32_t MaxIndices = MaxQuads * 6;

	Vertex* VertexBufferBase = nullptr;
	Vertex* VertexBufferPtr = nullptr;
	uint32_t IndexCount = 0;

	Ref<Texture2D> WhiteTexture;

	uint32_t VertexBuffer;

	Ref<Shader> QuadShader;
	int UniformViewProjLocation = -1;

	::ResourceQueue ResourceQueue;

	glm::vec4 VertexPositions[4];
	glm::vec2 VertexTexCoords[4];

	std::array<Ref<Texture2D>, 31> Textures;

	bool Wireframe = false;
};

static RendererData s_Data;

void Renderer::ExecuteRenderCommandQueue()
{
	ScopedInstrumentationTimer instrumentor("ExecuteRenderCommandQueue");
	Timer timer;
	//auto& queue = s_RenderCommandQueue[GetRenderQueueIndex()];
	auto& queue = s_RenderCommandQueue[0];
	for (auto& func : queue)
		func();

	queue.clear();
	std::cout << "ExecuteRenderCommandQueue took " << timer.ElapsedMillis() << std::endl;
}

void Renderer::Init()
{
	// Setup GL state
	Renderer::Submit([]()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	});

	std::string vertexSrc = R"(
		#version 450 core

		layout (location = 0) in vec3 a_Position;
		layout (location = 1) in vec4 a_Color;
		layout (location = 2) in vec2 a_TexCoord;
		layout (location = 3) in float a_TexID;

		out vec4 v_Color;
		out vec2 v_TexCoord;
		out float v_TexID;

		uniform mat4 u_ViewProj;
		uniform mat4 u_Transform;

		void main()
		{
			v_Color = a_Color;
			v_TexCoord = a_TexCoord;
			v_TexID = a_TexID;
			gl_Position = u_ViewProj * vec4(a_Position, 1.0);
		}
	)";

	std::string fragmentSrc = R"(
		#version 450 core

		layout (location = 0) out vec4 o_Color;

		in vec4 v_Color;
		in vec2 v_TexCoord;
		in float v_TexID;

		uniform vec4 u_Color;

		uniform sampler2D u_Textures[32];

		void main()
		{
			vec4 textureColor = texture(u_Textures[int(v_TexID)], v_TexCoord);
			o_Color = textureColor * v_Color;
		}
	)";

	s_Data.QuadShader = Ref<Shader>::Create(vertexSrc, fragmentSrc);
	s_Data.QuadShader->Bind();

	for (int i = 0; i < s_Data.Textures.size(); i++)
		s_Data.Textures[i] = nullptr;

	uint32_t whiteTextureData = 0xffffffff;
	s_Data.WhiteTexture = Ref<Texture2D>::Create(1, 1, &whiteTextureData);

	// Create Vertex Buffer
	s_Data.VertexBufferBase = new Vertex[s_Data.MaxVertices];

	Renderer::Submit([]()
	{
		s_Data.UniformViewProjLocation = glGetUniformLocation(s_Data.QuadShader->GetRendererID(), "u_ViewProj");

		auto loc = glGetUniformLocation(s_Data.QuadShader->GetRendererID(), "u_Textures");
		int texSlots[32];
		for (int i = 0; i < 32; i++)
			texSlots[i] = i;
		glUniform1iv(loc, 32, texSlots);

		GLuint va, ib;
		glCreateVertexArrays(1, &va);
		glBindVertexArray(va);

		glCreateBuffers(1, &s_Data.VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, s_Data.VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, s_Data.MaxVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexArrayAttrib(va, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

		glEnableVertexArrayAttrib(va, 1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

		glEnableVertexArrayAttrib(va, 2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord));

		glEnableVertexArrayAttrib(va, 3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, textureID));

		// Index Buffer
		uint32_t* indices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}
		glCreateBuffers(1, &ib);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_Data.MaxIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		delete[] indices;

		s_Data.VertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.VertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.VertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.VertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.VertexTexCoords[0] = { 0.0f, 0.0f };
		s_Data.VertexTexCoords[1] = { 1.0f, 0.0f };
		s_Data.VertexTexCoords[2] = { 1.0f, 1.0f };
		s_Data.VertexTexCoords[3] = { 0.0f, 1.0f };
	});
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height)
{
	Renderer::Submit([width, height]()
	{
		glViewport(0, 0, width, height);
	});
}

void Renderer::SetCamera(const Camera& camera)
{
	s_Data.QuadShader->Bind();
	Renderer::Submit([camera]()
	{
		glUniformMatrix4fv(s_Data.UniformViewProjLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewProj()));
	});
}

void Renderer::Clear(glm::vec4 color)
{
	Renderer::Submit([color]()
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	});
}

void Renderer::SetWireframe(bool wireframe)
{
	s_Data.Wireframe = wireframe;
}

void Renderer::ProcessResources()
{
	s_Data.ResourceQueue.Update();
}

ResourceQueue& Renderer::GetResourceQueue()
{
	return s_Data.ResourceQueue;
}

void Renderer::Begin()
{
	s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
	s_Data.IndexCount = 0;
}

void Renderer::End()
{
	s_Data.QuadShader->Bind();
	s_Data.WhiteTexture->Bind(0);

	for (size_t i = 0; i < s_Data.Textures.size(); i++)
	{
		if (s_Data.Textures[i])
		{
			s_Data.Textures[i]->Bind(i + 1);
			s_Data.Textures[i] = nullptr;
		}
	}

	auto vertexDataSize = ((uint8_t*)s_Data.VertexBufferPtr - (uint8_t*)s_Data.VertexBufferBase);
	Vertex* vertexData = new Vertex[vertexDataSize / sizeof(Vertex)];
	memcpy(vertexData, s_Data.VertexBufferBase, vertexDataSize);
	Renderer::Submit([
		vertexBuffer = s_Data.VertexBuffer,
		vertexData,
		vertexDataSize,
		wireframe = s_Data.Wireframe,
		indexCount = s_Data.IndexCount]()
	{
		glNamedBufferSubData(vertexBuffer, 0, vertexDataSize, vertexData);
		delete[] vertexData;

		glLineWidth(2.0f);
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	});
}

#if 0
void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color)
{
	s_Data.QuadShader->Bind();

	int transformLocation = glGetUniformLocation(s_Data.QuadShader->GetRendererID(), "u_Transform");
	if (transformLocation == -1)
		std::cout << "Could not find uniform!\n";

	int colorLocation = glGetUniformLocation(s_Data.QuadShader->GetRendererID(), "u_Color");
	if (colorLocation == -1)
		std::cout << "Could not find uniform!\n";

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), { 0, 0, 1 })
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4f(colorLocation, color.r, color.g, color.b, 1.0f);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
#else

void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
{
	float textureSlot = 0.0f;

	s_Data.VertexBufferPtr->position = { position.x - size.x * 0.5f,  position.y - size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 0.0f, 0.0f };
	s_Data.VertexBufferPtr->textureID = textureSlot;
	s_Data.VertexBufferPtr++;

	s_Data.VertexBufferPtr->position = { position.x + size.x * 0.5f,  position.y - size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 1.0f, 0.0f };
	s_Data.VertexBufferPtr->textureID = textureSlot;
	s_Data.VertexBufferPtr++;

	s_Data.VertexBufferPtr->position = { position.x + size.x * 0.5f,  position.y + size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 1.0f, 1.0f };
	s_Data.VertexBufferPtr->textureID = textureSlot;
	s_Data.VertexBufferPtr++;

	s_Data.VertexBufferPtr->position = { position.x - size.x * 0.5f,  position.y + size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 0.0f, 1.0f };
	s_Data.VertexBufferPtr->textureID = textureSlot;
	s_Data.VertexBufferPtr++;

	s_Data.IndexCount += 6;
}

void Renderer::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
		* glm::rotate(glm::mat4(1.0f), rotationRadians, { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	float textureSlot = 0.0f;

	for (int i = 0; i < 4; i++)
	{
		s_Data.VertexBufferPtr->position = transform * s_Data.VertexPositions[i];
		s_Data.VertexBufferPtr->color = color;
		s_Data.VertexBufferPtr->texCoord = s_Data.VertexTexCoords[i];
		s_Data.VertexBufferPtr->textureID = textureSlot;
		s_Data.VertexBufferPtr++;
	}

	s_Data.IndexCount += 6;
}

void Renderer::DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& color)
{
	DrawTexturedQuad(position, size, texture, s_Data.VertexTexCoords, color);
}

void Renderer::DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec2* texCoords, const glm::vec4& color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	float textureSlot = 0.0f;
	for (size_t i = 0; i < s_Data.Textures.size(); i++)
	{
		if (s_Data.Textures[i] == texture)
		{
			textureSlot = i + 1;
			break;
		}
	}

	if (textureSlot == 0.0f)
	{
		for (size_t i = 0; i < s_Data.Textures.size(); i++)
		{
			if (!s_Data.Textures[i])
			{
				s_Data.Textures[i] = texture;
				textureSlot = i + 1;
				break;
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		s_Data.VertexBufferPtr->position = transform * s_Data.VertexPositions[i];
		s_Data.VertexBufferPtr->color = color;
		s_Data.VertexBufferPtr->textureID = textureSlot;
		s_Data.VertexBufferPtr->texCoord = texCoords[i];
		s_Data.VertexBufferPtr++;
	}

	s_Data.IndexCount += 6;
}

void Renderer::DrawRotatedTexturedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, Texture2D* texture, const glm::vec4& color)
{
	DrawRotatedTexturedQuad(position, size, rotationRadians, texture, s_Data.VertexTexCoords, color);
}

void Renderer::DrawRotatedTexturedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, Texture2D* texture, const glm::vec2* texCoords, const glm::vec4& color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
		* glm::rotate(glm::mat4(1.0f), rotationRadians, { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	float textureSlot = 0.0f;
	for (size_t i = 0; i < s_Data.Textures.size(); i++)
	{
		if (s_Data.Textures[i] == texture)
		{
			textureSlot = i + 1;
			break;
		}
	}

	if (textureSlot == 0.0f)
	{
		for (size_t i = 0; i < s_Data.Textures.size(); i++)
		{
			if (!s_Data.Textures[i])
			{
				s_Data.Textures[i] = texture;
				textureSlot = i + 1;
				break;
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		s_Data.VertexBufferPtr->position = transform * s_Data.VertexPositions[i];
		s_Data.VertexBufferPtr->color = color;
		s_Data.VertexBufferPtr->texCoord = texCoords[i];
		s_Data.VertexBufferPtr->textureID = textureSlot;
		s_Data.VertexBufferPtr++;
	}

	s_Data.IndexCount += 6;
}

#endif