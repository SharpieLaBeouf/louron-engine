#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

// TIME TO MAKE A PONG CLONE


struct Paddle {

public:
	Paddle() { }

	Louron::TransformComponent transform;
	Louron::Material material;

	float speed = 2.5f;

};

struct Ball {

public:
	Ball() {
		transform.scale = glm::vec3(0.3f);
	}

	Louron::TransformComponent transform;
	Louron::Material material;

	float speed = 5.0f;

	glm::vec2 velocity = { speed, speed * -0.75f };

};


class Scene10 : public Scene {

private:

	Louron::Window& m_Window;
	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	Louron::Camera m_SceneCamera;
	Louron::Light m_Light;

	std::vector<std::unique_ptr<Paddle>> m_Paddles;

	std::unique_ptr<Ball> m_Ball;

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;

	float m_GameStartTimer = 3.0f;

	bool m_GameOver = false;
	bool m_GameRunning = false;
	bool m_FirstLoop = true;

public:


	Scene10() :
		m_Window(Louron::Engine::Get().GetWindow()),
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		m_SceneCamera(glm::vec3(0.0f, 10.0f, 0.0f))
	{
		std::cout << "[L20] Opening Scene 10 - PONG..." << std::endl;

		// Init Cube VAO
		{
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
		}


		// Load Standard Flat Shader
		m_ShaderLib.LoadShader("assets/Shaders/Materials/material_shader_phong.glsl");

		// Create Ball and Assign Shader to Ball Material and Set Colour to Red
		m_Ball = std::make_unique<Ball>();
		m_Ball->material.SetShader(m_ShaderLib.GetShader("material_shader_phong"));
		m_Ball->material.SetTexture(m_TextureLib.GetTexture("blank_texture"), Louron::L20_TEXTURE_DIFFUSE_MAP);
		m_Ball->material.SetDiffuse({ 1.0f, 0.0f, 0.0f, 1.0f });

		// Create Paddles 
		m_Paddles.push_back(std::make_unique<Paddle>());
		m_Paddles.push_back(std::make_unique<Paddle>());

		// Set Positions of Individual Paddles
		m_Paddles[0]->transform.position = glm::vec3(0.0f, 0.0f, -8.5f);
		m_Paddles[1]->transform.position = glm::vec3(0.0f, 0.0f, 8.5f);

		// Assign Material Values and Modify Scale
		int index = 0;
		for (auto& paddle : m_Paddles) {
			paddle->material.SetShader(m_ShaderLib.GetShader("material_shader_phong"));
			paddle->material.SetDiffuse({ 0.41f, 0.41f, 0.41f, 1.0f });
			paddle->transform.scale = glm::vec3(3.5f, 1.0f, 1.0f);
		}

		m_Light.position.y = 1.0f;
		m_Light.specular = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

		m_SceneCamera.setPitch(-90.0f);
		m_SceneCamera.setYaw(-180.0f);
		m_SceneCamera.toggleMovement();
	}

	~Scene10() {
		std::cout << "[L20] Closing Scene 10 - PONG..." << std::endl;


	}

	void OnAttach() override {
		glEnable(GL_DEPTH_TEST);
		lastTime = (float)glfwGetTime();
		ResetGame();
	}

	void OnDetach() override {

		glDisable(GL_DEPTH_TEST);
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		m_SceneCamera.Update(deltaTime);

		if (!m_GameOver) {

			// COUNT DOWN: If game is not over, and game is not running, conduct game start timer of 3 seconds!
			if (!m_GameRunning) {
				if (m_GameStartTimer <= 0.0f) {
					m_GameRunning = true;
				}
				else m_GameStartTimer -= deltaTime;
			}

			// GAME LOOP: If timer is up, start the game!
			else if (m_GameRunning) {

				// TO REMOVE: Manual Pong
				if (m_Input.GetKeyDown(GLFW_KEY_O))
					m_Ball->velocity.x = -m_Ball->velocity.x;

				if (m_Input.GetKeyDown(GLFW_KEY_P))
					m_Ball->velocity.y = -m_Ball->velocity.y;

				// Process Ball Movement
				ProcessBallMovement();

				// Process Paddle Movement
				ProcessPaddleMovement();

				ProcessCollisions();

				// TO REMOVE: Manual Game Over
				if (m_Input.GetKeyDown(GLFW_KEY_F1))
					m_GameOver = true;

			}
		}

		Draw();
	}

	void UpdateGUI() override {

		if (!m_GameOver) {

			// COUNT DOWN: Display countdown in GUI if the game is not over, and the game is not running.
			if (!m_GameRunning) {
				if (m_GameStartTimer >= 0.0f) {
					ImGui::Begin("Game Start Countdown!", (bool*)0,
						ImGuiWindowFlags_NoMove |
						ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_NoSavedSettings |
						ImGuiWindowFlags_NoTitleBar |
						ImGuiWindowFlags_NoCollapse);

					float gameRulesWidth = 300.0f;
					ImGui::SetWindowPos(ImVec2(Louron::Engine::Get().GetWindow().GetWidth() / 2 - gameRulesWidth / 2, 10.0f));
					ImGui::SetWindowSize(ImVec2(gameRulesWidth, 10.0f));

					ImGui::Text("Game Starting In %.1f Seconds!", m_GameStartTimer);

					ImGui::End();
				}
			}

			// GAME LOOP: If timer is up, start the game!
			else if (m_GameRunning) {

				ImGui::Begin("Game Loop GUI", (bool*)0,
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoCollapse);

				float gameRulesWidth = 300.0f;
				ImGui::SetWindowPos(ImVec2(Louron::Engine::Get().GetWindow().GetWidth() / 2 - gameRulesWidth / 2, 10.0f));
				ImGui::SetWindowSize(ImVec2(gameRulesWidth, 10.0f));

				ImGui::Text("Game is In Session!");

				ImGui::End();
			}
		}
		// GAME OVER GUI
		else {

			ImGui::Begin("Game Loop GUI", (bool*)0,
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoCollapse);

			float gameRulesWidth = 300.0f;
			ImGui::SetWindowPos(ImVec2(Louron::Engine::Get().GetWindow().GetWidth() / 2 - gameRulesWidth / 2, 10.0f));
			ImGui::SetWindowSize(ImVec2(gameRulesWidth, 10.0f));

			if (ImGui::Button("Reset Game?"))
				ResetGame();

			ImGui::End();


		}

	}

private:

	void ProcessCollisions() {

	}

	void ProcessBallMovement() {
		m_Ball->transform.position.z += deltaTime * m_Ball->velocity.x;
		m_Ball->transform.position.x += deltaTime * m_Ball->velocity.y;
		m_Light.position = glm::vec3(m_Ball->transform.position.x, 1.0f, m_Ball->transform.position.z);
	}

	void ProcessPaddleMovement() {

		// Move Paddle 1 Up and Down
		if (m_Input.GetKey(GLFW_KEY_W)) {
			m_Paddles[0]->transform.position.x += deltaTime * m_Paddles[0]->speed;
		}
		if (m_Input.GetKey(GLFW_KEY_S)) {
			m_Paddles[0]->transform.position.x -= deltaTime * m_Paddles[0]->speed;
		}

		// Move Paddle 2 Up and Down
		if (m_Input.GetKey(GLFW_KEY_UP)) {
			m_Paddles[1]->transform.position.x += deltaTime * m_Paddles[1]->speed;
		}
		if (m_Input.GetKey(GLFW_KEY_DOWN)) {
			m_Paddles[1]->transform.position.x -= deltaTime * m_Paddles[1]->speed;
		}
	}

	void ResetGame() {
		m_GameOver = false;
		m_GameRunning = false;
		m_GameStartTimer = 3.0f;

		// Reset Ball, Light, and Paddles
		m_Ball->velocity = { m_Ball->speed, m_Ball->speed * -0.75f };
		m_Ball->transform.position = glm::vec3(0.0f);
		m_Light.position = glm::vec3(m_Ball->transform.position.x, 1.0f, m_Ball->transform.position.z);
		for (auto& paddle : m_Paddles) paddle->transform.position.x = 0.0f;

	}

	void Draw() override {
		// Set Fixed Window Size for Pong
		glfwSetWindowSize((GLFWwindow*)m_Window.GetNativeWindow(), 1600, 900);

		glClearColor(211.0f / 255.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f);

		glBindVertexArray(cube_VAO);

		// Render Ball
		if (m_Ball->material.Bind()) {

			m_Ball->material.SetUniforms();
			m_Ball->material.GetShader()->SetMat4("model", m_Ball->transform);
			m_Ball->material.GetShader()->SetMat4("proj", proj);
			m_Ball->material.GetShader()->SetMat4("view", m_SceneCamera.getViewMatrix());
			m_Ball->material.GetShader()->SetVec3("u_Light.position", m_Light.position);
			m_Ball->material.GetShader()->SetVec4("u_Light.ambient", m_Light.ambient);
			m_Ball->material.GetShader()->SetVec4("u_Light.diffuse", m_Light.diffuse);
			m_Ball->material.GetShader()->SetVec4("u_Light.specular", m_Light.specular);
			m_Ball->material.GetShader()->SetVec3("u_CameraPos", m_SceneCamera.getPosition());

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			m_Ball->material.UnBind();
		}

		// Render Paddles
		for (const auto& paddle : m_Paddles) {
			if (paddle->material.Bind()) {

				paddle->material.SetUniforms();
				paddle->material.GetShader()->SetMat4("model", paddle->transform);
				paddle->material.GetShader()->SetMat4("proj", proj);
				paddle->material.GetShader()->SetMat4("view", m_SceneCamera.getViewMatrix());
				paddle->material.GetShader()->SetVec3("u_Light.position", m_Light.position);
				paddle->material.GetShader()->SetVec4("u_Light.ambient", m_Light.ambient);
				paddle->material.GetShader()->SetVec4("u_Light.diffuse", m_Light.diffuse);
				paddle->material.GetShader()->SetVec4("u_Light.specular", m_Light.specular);
				paddle->material.GetShader()->SetVec3("u_CameraPos", m_SceneCamera.getPosition());

				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				m_Ball->material.UnBind();
			}
		}

	}

	// Private Cube Data
private:

	GLuint cube_VAO = -1;
	GLuint cube_VBO = -1;
	GLuint cube_EBO = -1;
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