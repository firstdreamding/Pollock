#include "Renderer.h"

#include <iostream>

#include "glad/glad.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

struct RendererData
{
	const uint32_t MaxQuads = 100000;
	const uint32_t MaxVertices = MaxQuads * 4;
	const uint32_t MaxIndices = MaxQuads * 6;

	Vertex* VertexBufferBase = nullptr;
	Vertex* VertexBufferPtr = nullptr;
	uint32_t IndexCount = 0;

	uint32_t VertexBuffer;

	std::unique_ptr<Shader> QuadShader;
	int UniformViewProjLocation = -1;

	glm::vec4 VertexPositions[4];
	glm::vec2 VertexTexCoords[4];

	bool Wireframe = false;
};

static RendererData s_Data;

void Renderer::Init()
{
	std::string vertexSrc = R"(
		#version 450 core

		layout (location = 0) in vec3 a_Position;
		layout (location = 1) in vec4 a_Color;
		layout (location = 2) in vec2 a_TexCoord;

		out vec4 v_Color;
		out vec2 v_TexCoord;

		uniform mat4 u_ViewProj;
		uniform mat4 u_Transform;

		void main()
		{
			v_Color = a_Color;
			v_TexCoord = a_TexCoord;
			gl_Position = u_ViewProj * vec4(a_Position, 1.0);
		}
	)";

	std::string fragmentSrc = R"(
		#version 450 core

		layout (location = 0) out vec4 o_Color;

		in vec4 v_Color;
		in vec2 v_TexCoord;

		uniform vec4 u_Color;

		uniform sampler2D u_Texture;

		void main()
		{		
			// o_Color = vec4(v_TexCoord, 0.0, 1.0);//v_Color;
			o_Color = texture(u_Texture, v_TexCoord) * v_Color;
		}
	)";

	s_Data.QuadShader = std::make_unique<Shader>(vertexSrc, fragmentSrc);
	s_Data.QuadShader->Bind();
	s_Data.UniformViewProjLocation = glGetUniformLocation(s_Data.QuadShader->GetRendererID(), "u_ViewProj");

	auto loc = glGetUniformLocation(s_Data.QuadShader->GetRendererID(), "u_Texture");
	glUniform1i(loc, 0);

	// Create Vertex Buffer
	s_Data.VertexBufferBase = new Vertex[s_Data.MaxVertices];

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
	s_Data.VertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
	s_Data.VertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
	s_Data.VertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

	s_Data.VertexTexCoords[0] = { 0.0f, 0.0f };
	s_Data.VertexTexCoords[1] = { 1.0f, 0.0f };
	s_Data.VertexTexCoords[2] = { 1.0f, 1.0f };
	s_Data.VertexTexCoords[3] = { 0.0f, 1.0f };
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height)
{
	glViewport(0, 0, width, height);
}

void Renderer::SetCamera(const Camera& camera)
{
	s_Data.QuadShader->Bind();
	glUniformMatrix4fv(s_Data.UniformViewProjLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewProj()));
}

void Renderer::Clear(glm::vec4& clearColor)
{
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetWireframe(bool wireframe)
{
	s_Data.Wireframe = wireframe;
}

void Renderer::Begin()
{
	s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
	s_Data.IndexCount = 0;
}

void Renderer::End()
{
	auto size = ((uint8_t*)s_Data.VertexBufferPtr - (uint8_t*)s_Data.VertexBufferBase);
	glNamedBufferSubData(s_Data.VertexBuffer, 0, size, s_Data.VertexBufferBase);

	s_Data.QuadShader->Bind();
	glLineWidth(2.0f);
	if (s_Data.Wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, s_Data.IndexCount, GL_UNSIGNED_INT, nullptr);
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
	s_Data.VertexBufferPtr->position = { position.x - size.x * 0.5f,  position.y - size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 0.0f, 0.0f };
	s_Data.VertexBufferPtr++;

	s_Data.VertexBufferPtr->position = { position.x + size.x * 0.5f,  position.y - size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 1.0f, 0.0f };
	s_Data.VertexBufferPtr++;

	s_Data.VertexBufferPtr->position = { position.x + size.x * 0.5f,  position.y + size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 1.0f, 1.0f };
	s_Data.VertexBufferPtr++;

	s_Data.VertexBufferPtr->position = { position.x - size.x * 0.5f,  position.y + size.y * 0.5f, 0.0f };
	s_Data.VertexBufferPtr->color = color;
	s_Data.VertexBufferPtr->texCoord = { 0.0f, 1.0f };
	s_Data.VertexBufferPtr++;

	s_Data.IndexCount += 6;
}

void Renderer::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, 0.0f })
		* glm::rotate(glm::mat4(1.0f), rotationRadians, { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	for (int i = 0; i < 4; i++)
	{
		s_Data.VertexBufferPtr->position = transform * s_Data.VertexPositions[i];
		s_Data.VertexBufferPtr->color = color;
		s_Data.VertexBufferPtr->texCoord = s_Data.VertexTexCoords[i];
		s_Data.VertexBufferPtr++;
	}

	s_Data.IndexCount += 6;
}

#endif