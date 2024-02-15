#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene2 : public TestScene {

//Private Setup Variables
private:

	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

//Constructors
public:

	Scene2() : 
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()) ,
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary())
	{
		std::cout << "[L20] Loading Scene 2..." << std::endl;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}
	~Scene2() override
	{
		std::cout << "[L20] Unloading Scene 2..." << std::endl;
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}


//Private Scene Variables
private:
	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
	int speed = 1;

	glm::vec4 back_colour = glm::vec4( 0.992f, 0.992f, 0.588f, 1.0f );
	glm::vec4 fore_colour = glm::vec4( 1.0f  , 1.0f  , 1.0f  , 1.0f );

	Louron::Transform trans;
		
//Public Functions
public:

	void Update() override {
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Draw();
	}
	void UpdateGUI() override {

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

		ImGui::InputInt("Speed", &speed);

		glm::vec3 temp = trans.GetPosition();
		ImGui::DragFloat3("Translate", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f");
		trans.SetPosition(temp);

		temp = trans.GetRotation();
		ImGui::DragFloat3("Rotate", glm::value_ptr(temp), 1.0f, 0, 0, "%.2f");
		trans.SetRotation(temp);

		temp = trans.GetScale();
		ImGui::DragFloat3("Scale", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f");
		trans.SetScale(temp);

		ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

		ImGui::Text("F11 = Toggle Fullscreen");

		static bool wireFrame = false;
		ImGui::Checkbox("Wireframe Mode", &wireFrame);

		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ImGui::Separator();

		ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
		ImGui::ColorPicker4("Triangles", glm::value_ptr(fore_colour));

		ImGui::End();
	}

private:

	void Draw() override {
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		trans.RotateY(deltaTime * speed);

		std::shared_ptr<Louron::Shader> shader = m_ShaderLib.GetShader("basic_texture");
		if (shader)
		{
			glBindVertexArray(VAO);

			shader->Bind();
			shader->SetMat4("u_VertexIn.Proj", glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f));
			shader->SetMat4("u_VertexIn.View", glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
			shader->SetMat4("u_VertexIn.Model", trans);
			shader->SetVec4("u_OurColour", fore_colour);

			shader->SetInt("u_OurTexture", 0);
			glActiveTexture(GL_TEXTURE0);
			m_TextureLib.GetTexture("cube_texture")->Bind();

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			m_TextureLib.UnBind();
		}
		shader->UnBind();
	}

	unsigned int VAO = -1;
	unsigned int VBO = -1;
	unsigned int EBO = -1;

	float vertices[20] = {
			0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
			0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left 
	};
	unsigned int indices[6] = {
		0, 1, 3,
		1, 2, 3
	};

};
