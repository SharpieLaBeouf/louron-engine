#pragma once

#include <iostream>
#include <stack>

#include <imgui/imgui.h>

#include "Louron.h"
#include "Test Scene Base.h"

class Scene6 : public Scene {

	//Private Setup Variables
private:

	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	Louron::Material* flat_cube_mat = nullptr;
	Louron::Material* box_cube_mat = nullptr;
	Louron::Material* stone_cube_mat = nullptr;
	Louron::Light light_properties;

	Louron::Camera* m_SceneCamera;

public:

	Scene6() :
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary())
	{
		std::cout << "[L20] Opening Scene 6..." << std::endl;

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


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_SceneCamera = new Louron::Camera({ 0.0f, 1.0f, 5.0f });
		m_SceneCamera->MouseToggledOff = false;
		m_SceneCamera->MovementSpeed = 10.0f;
		m_SceneCamera->MovementYDamp = 0.65f;

		m_ShaderLib.LoadShader("assets/Shaders/Materials/material_shader_flat.glsl");
		m_ShaderLib.LoadShader("assets/Shaders/Materials/material_shader_phong.glsl");

		m_TextureLib.loadTexture("assets/Images/cube_texture.png");
		m_TextureLib.loadTexture("assets/Images/cube_texture_specular.png");
		m_TextureLib.loadTexture("assets/Images/stone_texture.png");
		m_TextureLib.loadTexture("assets/Images/stone_texture_specular.png");

		flat_cube_mat = new Louron::Material(m_ShaderLib.GetShader("material_shader_flat"), m_TextureLib.GetTexture("blank_texture"));
		flat_cube_mat->Bind();
		flat_cube_mat->SetDiffuse(glm::vec4(1.0f));
		
		box_cube_mat = new Louron::Material(m_ShaderLib.GetShader("material_shader_phong"), m_TextureLib.GetTexture("blank_texture"));
		box_cube_mat->Bind();
		box_cube_mat->SetDiffuse(glm::vec4(1.0f));
		box_cube_mat->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib.GetTexture("cube_texture"));
		box_cube_mat->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_SPECULAR_MAP, m_TextureLib.GetTexture("cube_texture_specular"));
		
		stone_cube_mat = new Louron::Material(m_ShaderLib.GetShader("material_shader_phong"), m_TextureLib.GetTexture("blank_texture"));
		stone_cube_mat->Bind();
		stone_cube_mat->SetDiffuse(glm::vec4(1.0f));
		stone_cube_mat->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib.GetTexture("stone_texture"));
		stone_cube_mat->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_SPECULAR_MAP, m_TextureLib.GetTexture("stone_texture_specular"));
		
		light_properties.position.y = -1.0f;
		light_properties.position.z = -3.5f;	
	}

	~Scene6() override
	{
		std::cout << "[L20] Closing Scene 6..." << std::endl;

		glDeleteVertexArrays(1, &cube_VAO);
		glDeleteBuffers(1, &cube_VBO);
		glDeleteBuffers(1, &cube_EBO);

		delete flat_cube_mat;
		delete box_cube_mat;
		delete stone_cube_mat;
		delete m_SceneCamera;
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

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

		if (ImGui::TreeNode("Flat Cube Material"))
		{
			ImGui::ColorEdit4("Ambient", glm::value_ptr(*flat_cube_mat->GetAmbient()));
			ImGui::ColorEdit4("Diffuse", glm::value_ptr(*flat_cube_mat->GetDiffuse()));
			ImGui::ColorEdit4("Specular", glm::value_ptr(*flat_cube_mat->GetSpecular()));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Box Cube Material"))
		{
			ImGui::ColorEdit4("Ambient", glm::value_ptr(*box_cube_mat->GetAmbient()));
			ImGui::ColorEdit4("Diffuse", glm::value_ptr(*box_cube_mat->GetDiffuse()));
			ImGui::ColorEdit4("Specular", glm::value_ptr(*box_cube_mat->GetSpecular()));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Stone Cube Material"))
		{
			ImGui::ColorEdit4("Ambient", glm::value_ptr(*stone_cube_mat->GetAmbient()));
			ImGui::ColorEdit4("Diffuse", glm::value_ptr(*stone_cube_mat->GetDiffuse()));
			ImGui::ColorEdit4("Specular", glm::value_ptr(*stone_cube_mat->GetSpecular()));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Light Properties"))
		{
			ImGui::DragFloat3("Position", glm::value_ptr(light_properties.position), 0.1f);
			ImGui::ColorEdit4("Ambient", glm::value_ptr(light_properties.ambient));
			ImGui::ColorEdit4("Diffuse", glm::value_ptr(light_properties.diffuse));
			ImGui::ColorEdit4("Specular", glm::value_ptr(light_properties.specular));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Colours"))
		{
			ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
			ImGui::TreePop();
		}
		ImGui::End();
	}

private:

	void Draw() override {

		glEnable(GL_DEPTH_TEST);
		glClearColor(back_colour[0], back_colour[1], back_colour[2], back_colour[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(cube_VAO);
		if (flat_cube_mat->Bind())
		{
			flat_cube_mat->SetUniforms();
			flat_cube_mat->GetShader()->SetMat4("model", glm::translate(glm::mat4(1.0f), light_properties.position));
			flat_cube_mat->GetShader()->SetMat4("proj", glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f));
			flat_cube_mat->GetShader()->SetMat4("view", m_SceneCamera->getViewMatrix()); 
			
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			flat_cube_mat->UnBind();
		}

		if (stone_cube_mat->Bind()) {
			stone_cube_mat->SetUniforms();
			stone_cube_mat->GetShader()->SetMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));
			stone_cube_mat->GetShader()->SetMat4("proj", glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f));
			stone_cube_mat->GetShader()->SetMat4("view", m_SceneCamera->getViewMatrix());
			stone_cube_mat->GetShader()->SetVec3("u_Light.position", light_properties.position);
			stone_cube_mat->GetShader()->SetVec4("u_Light.ambient", light_properties.ambient);
			stone_cube_mat->GetShader()->SetVec4("u_Light.diffuse", light_properties.diffuse);
			stone_cube_mat->GetShader()->SetVec4("u_Light.specular", light_properties.specular);
			stone_cube_mat->GetShader()->SetVec3("u_CameraPos", m_SceneCamera->getPosition());

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			stone_cube_mat->UnBind();
		}

		if (box_cube_mat->Bind()) {
			box_cube_mat->SetUniforms();
			box_cube_mat->GetShader()->SetMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));
			box_cube_mat->GetShader()->SetMat4("proj", glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f));
			box_cube_mat->GetShader()->SetMat4("view", m_SceneCamera->getViewMatrix());
			box_cube_mat->GetShader()->SetVec3("u_Light.position", light_properties.position);
			box_cube_mat->GetShader()->SetVec4("u_Light.ambient", light_properties.ambient);
			box_cube_mat->GetShader()->SetVec4("u_Light.diffuse", light_properties.diffuse);
			box_cube_mat->GetShader()->SetVec4("u_Light.specular", light_properties.specular);
			box_cube_mat->GetShader()->SetVec3("u_CameraPos", m_SceneCamera->getPosition());

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			box_cube_mat->UnBind();
		}

		glDisable(GL_DEPTH_TEST);
	}

	glm::vec4 back_colour = { 0.3137f, 0.7843f, 1.0f, 1.0f };
	glm::vec4 cube_colour = { 1.0f, 1.0f, 1.0f, 1.0f };

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;

	int selection = 0;

	GLuint cube_VAO;
	GLuint cube_VBO;
	GLuint cube_EBO;
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

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // right top right
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // right bottom right
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // right bottom left
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // right top left 

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // left top right
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // left bottom right
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // left bottom left
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // left top left 

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
};