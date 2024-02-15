#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene5 : public TestScene {

	//Private Setup Variables
private:

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;

	unsigned int light_VAO = -1;
	unsigned int cube_VAO = -1;
	unsigned int cube_VBO = -1;
	unsigned int cube_EBO = -1;
	float cube_vertices[192] = {
		// POSITION          // NORMAL            // TEXCORD
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  // front top right
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  // front bottom right
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  // front bottom left
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  // front top left
								  		 	    	   
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  // back top right
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  // back bottom right
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  // back bottom left
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  // back top left
								   		 	    	   
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // right top right
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // right bottom right
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // right bottom left
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // right top left 
								  		 	    	   
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // left top right
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // left bottom right
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // left bottom left
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // left top left 
								   		 	    	   
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  // top top right
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  // top bottom right
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  // top bottom left
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  // top top left 
								   		 	    	   
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  // bottom top right
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  // bottom bottom right
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  // bottom bottom left
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f   // bottom top left 
	};
	unsigned int cube_indices[36] = {
		0, 1, 3,
		1, 2, 3,

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,

		12, 13, 15,
		13, 14, 15,

		16, 17, 19,
		17, 18, 19,

		20, 21, 23,
		21, 22, 23
	};

public:

	Scene5() :
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary())
	{
		std::cout << "[L20] Loading Scene 5..." << std::endl;

		// Init Cube VAO
		glGenVertexArrays(1, &cube_VAO);
		glGenBuffers(1, &cube_VBO);
		glGenBuffers(1, &cube_EBO);

		glBindVertexArray(cube_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(2);
			
		// Init Light VAO
		glGenVertexArrays(1, &light_VAO);
		glBindVertexArray(light_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
		glEnableVertexAttribArray(0);	

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_SceneCamera = new Louron::Camera(glm::vec3(0.0f, 3.0f, 10.0f));
		m_SceneCamera->MouseToggledOff = false;
		m_SceneCamera->MovementSpeed = 10.0f;
		m_SceneCamera->MovementYDamp = 0.65f;

		light_trans.SetPosition({ 0.0f, 10.0f, 0.0f });
		cube_trans.SetScale({ 5.0f, 1.0f, 5.0f });
							
	}
	~Scene5() override
	{
		std::cout << "[L20] Unloading Scene 5..." << std::endl;
		glDisable(GL_DEPTH_TEST);
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		glDeleteVertexArrays(1, &light_VAO);
		glDeleteVertexArrays(1, &cube_VAO);
		glDeleteBuffers(1, &cube_VBO);
		glDeleteBuffers(1, &cube_EBO);

		delete m_SceneCamera;
	}

public:


	void Update() override {
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		//light_trans.position = glm::vec3(0.0f, 10.0f, 0.0f);
		light_trans.SetPosition({
			sin(currentTime) * rows * cube_trans.GetScale().x / 2.0f,
			10.0f,
			glm::clamp(
				tan(currentTime) * cube_trans.GetScale().z,
				-(float)col * cube_trans.GetScale().z / 2.0f,
				 (float)col * cube_trans.GetScale().z / 2.0f) });
		
		m_SceneCamera->Update(deltaTime);

		Draw();
	}
	void UpdateGUI() override {

		static bool wireFrame = false;

		ImGui::Begin("Scene Control", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

		ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(300.0f, 400.0f));

		ImGui::Text("F11 = Toggle Fullscreen");
		ImGui::Checkbox("Wireframe Mode", &wireFrame);

		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ImGui::Separator();

		ImGui::DragInt("Rows", &rows, 0.1f);
		ImGui::DragInt("Columns", &col, 0.1f);

		ImGui::Separator();

		if (ImGui::TreeNode("Cube Transform"))
		{
			glm::vec3 temp = cube_trans.GetPosition();
			ImGui::DragFloat3("Translate", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f");
			cube_trans.SetPosition(temp);

			temp = cube_trans.GetRotation();
			ImGui::DragFloat3("Rotate", glm::value_ptr(temp), 1.0f, 0, 0, "%.2f");
			cube_trans.SetRotation(temp);

			temp = cube_trans.GetScale();
			ImGui::DragFloat3("Scale", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f");
			cube_trans.SetScale(temp);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Colours"))
		{
			ImGui::ColorPicker4("Background", back_colour);
			ImGui::ColorPicker4("Box", box_colour);
			ImGui::TreePop();
		}
		ImGui::End();
	}

private:

	float box_colour[4] = { 0.992f, 0.325f, 0.325f, 1.0f };
	float back_colour[4] = { 120.0f / 255.0f, 200.0f / 255.0f, 255.0f, 1.0f };

	Louron::Transform cube_trans;
	Louron::Transform light_trans;

	Louron::Camera* m_SceneCamera = nullptr;


	float currentTime = 0;
	float lastTime = 0;
	float deltaTime = 0;
	int rows = 9, col = 9;

private:

	void Draw() override {

		glEnable(GL_DEPTH_TEST);
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = m_SceneCamera->GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f);

		// Draw Light Source
		std::shared_ptr<Louron::Shader> shader = m_ShaderLib.GetShader("basic");
		if (shader) {
			glBindVertexArray(light_VAO);

			shader->Bind();
			shader->SetMat4("u_VertexIn.View", view);
			shader->SetMat4("u_VertexIn.Proj", proj);
			shader->SetMat4("u_VertexIn.Model", light_trans);
			shader->SetVec4("u_OurColour", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// Draw Cube
		shader = m_ShaderLib.GetShader("basic_phong");
		if (shader) {
			glBindVertexArray(cube_VAO);

			shader->Bind();
			shader->SetMat4("u_VertexIn.View", view);
			shader->SetMat4("u_VertexIn.Proj", proj);
			shader->SetMat3("u_NormalToWorld", glm::mat3(glm::transpose(glm::inverse(cube_trans.GetTransform()))));
			shader->SetVec3("u_ViewPos", m_SceneCamera->GetPosition());
			shader->SetVec3("u_LightPos", light_trans.GetPosition());
			shader->SetVec4("u_LightColour", glm::vec4(1.0f));

			glm::vec3 pos = glm::vec3(0.0f);

			for (int x = 0; x < rows; x++)
			{
				pos.x = (float)(x - rows / 2);
				for (int z = 0; z < col; z++)
				{
					pos.z = (float)(z - col / 2);

					double time = glfwGetTime();
					pos.y = (float)sin((time * 8 + floor(x - 9) + floor(z - 9))) / 9 * 9;

					if (z % 2 != 0 || x % 2 != 0) {
						shader->SetVec4("u_TargetColour", glm::vec4(box_colour[0], box_colour[1], box_colour[2], box_colour[3]));
					}
					else {
						shader->SetVec4("u_TargetColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					}

					shader->SetMat4("u_VertexIn.Model", glm::translate(cube_trans.GetTransform(), pos));
					glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				}
			}
		}
		m_ShaderLib.UnBindAllShaders();

		glDisable(GL_DEPTH_TEST);
	}
};