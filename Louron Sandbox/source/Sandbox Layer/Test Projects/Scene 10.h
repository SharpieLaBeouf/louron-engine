#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include "glm/gtc/random.hpp"

// TIME TO MAKE A PONG CLONE

struct Paddle {

public:
	Paddle() { }

	Louron::Transform transform;
	Louron::Material material;

	float speed = 5.0f;

	int score = 0;
};

struct Ball {

public:
	Ball() {
		transform.SetScale(glm::vec3(0.3f));
	}

	Louron::Transform transform;
	Louron::Material material;

	float speed = 16.0f;

	glm::vec2 velocity = glm::vec2(speed, 0.0f);

};

class Scene10 : public TestScene {

private:

	Louron::Window& m_Window;
	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	Louron::Camera m_SceneCamera;
	ManualLight m_Light;

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
		std::cout << "[L20] Loading Scene 10..." << std::endl;

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

		// Create Ball and Assign Shader to Ball Material and Set Colour to Red
		m_Ball = std::make_unique<Ball>();
		m_Ball->material.SetShader(m_ShaderLib.GetShader("material_shader_phong"));
		m_Ball->material.SetTexture(m_TextureLib.GetTexture("blank_texture"), Louron::L20_TEXTURE_DIFFUSE_MAP);
		m_Ball->material.SetDiffuse({ 1.0f, 0.0f, 0.0f, 1.0f });

		// Create Paddles 
		m_Paddles.push_back(std::make_unique<Paddle>());
		m_Paddles.push_back(std::make_unique<Paddle>());

		// Set Positions of Individual Paddles
		m_Paddles[0]->transform.SetPosition(glm::vec3(0.0f, 0.0f, -8.5f));
		m_Paddles[1]->transform.SetPosition(glm::vec3(0.0f, 0.0f, 8.5f));

		// Assign Material Values and Modify Scale
		int index = 0;
		for (auto& paddle : m_Paddles) {
			paddle->material.SetShader(m_ShaderLib.GetShader("material_shader_phong"));
			paddle->material.SetDiffuse({ 0.41f, 0.41f, 0.41f, 1.0f });
			paddle->transform.SetScale(glm::vec3(2.0f, 1.0f, 1.0f));
		}

		m_Light.position.y = 1.0f;
		m_Light.specular = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

		m_SceneCamera.setPitch(-90.0f);
		m_SceneCamera.setYaw(-180.0f);
		m_SceneCamera.toggleMovement();
	}

	~Scene10() {
		std::cout << "[L20] Unloading Scene 10+..." << std::endl;


	}

	void OnAttach() override {
		glEnable(GL_DEPTH_TEST);
		lastTime = (float)glfwGetTime();
		ResetGame();

		// Set Fixed Window Size for Pong
		glfwSetWindowSize((GLFWwindow*)m_Window.GetNativeWindow(), 1600, 900);
		glfwSetWindowAttrib((GLFWwindow*)m_Window.GetNativeWindow(), GLFW_RESIZABLE, GL_FALSE);
	}

	void OnDetach() override {

		glDisable(GL_DEPTH_TEST);
		glfwSetWindowAttrib((GLFWwindow*)m_Window.GetNativeWindow(), GLFW_RESIZABLE, GL_TRUE);
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		m_SceneCamera.Update(deltaTime);

		if (!m_GameOver) {

			// COUNT DOWN: If game is not over, and game is not running, conduct game start timer of 3 seconds!
			if (!m_GameRunning) {
				if (m_GameStartTimer <= 0.0f)
					m_GameRunning = true;
				else 
					m_GameStartTimer -= deltaTime;
			}

			// GAME LOOP: If timer is up, start the game!
			else if (m_GameRunning) {

				// Process Ball Movement
				ProcessBallMovement();

				// Process Paddle Movement
				ProcessPaddleMovement();

				// Process Collissions
				ProcessCollisions();

				// Check if Game is Over
				if(m_Paddles[0]->score >= 3 || m_Paddles[1]->score >= 3)
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
				ImGui::SetWindowSize(ImVec2(gameRulesWidth, 100.0f));

				ImGui::Text("Game is In Session!");
				ImGui::Text("Player 1 Score: %i", m_Paddles[0]->score);
				ImGui::Text("Player 2 Score: %i", m_Paddles[1]->score);

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
			ImGui::SetWindowSize(ImVec2(gameRulesWidth, 100.0f));

			if (m_Paddles[0]->score >= 3)
				ImGui::Text("Player 1 Won!");

			if (m_Paddles[1]->score >= 3)
				ImGui::Text("Player 2 Won!");

			ImGui::Text("Final Score: %i - %i", m_Paddles[0]->score, m_Paddles[1]->score);

			if (ImGui::Button("Reset Game?"))
				ResetGame();

			ImGui::End();


		}

	}

private:

	enum CollisionType{ None, Top, Bottom, Right, Left };

	void ScorePoint(std::unique_ptr<Paddle>& paddle) {

		paddle->score++;

		m_Ball->velocity = { m_Ball->speed, 0.0f };
		m_Ball->transform.SetPosition(glm::vec3(0.0f));
		m_Light.position = glm::vec3(m_Ball->transform.GetPosition().x, 1.0f, m_Ball->transform.GetPosition().z);
		
		for (auto& paddle : m_Paddles) 
			paddle->transform.SetPositionX(0.0f);
	}

	void ProcessCollisions() {

		if (CheckPaddleCollision(m_Paddles[0]) || CheckPaddleCollision(m_Paddles[1])) {

			Louron::Audio::Get().PlaySound("assets/Audio/bleep.mp3", false);

			if (m_Ball->velocity.y == 0.0f) {
				m_Ball->velocity.y = m_Ball->speed * 0.6f;
			} else {
				if (glm::linearRand(0, 1) == 1) {
					m_Ball->velocity.y = -m_Ball->velocity.y;
				}
			}
			m_Ball->velocity.x = -m_Ball->velocity.x;
		}

		switch (CheckWallCollision()) {
			case CollisionType::Top:
				m_Ball->velocity.y = -m_Ball->velocity.y;
			break;
			case CollisionType::Bottom:
				m_Ball->velocity.y = -m_Ball->velocity.y;
			break;
			case CollisionType::Left:
				ScorePoint(m_Paddles[1]);
			break;
			case CollisionType::Right:
				ScorePoint(m_Paddles[0]);
			break;
		}
		
	}

	// TODO: Fix Paddle Collision Bugs

	bool CheckPaddleCollision(const std::unique_ptr<Paddle>& paddle) {

		float ballLeft		= m_Ball->transform.GetPosition().z - m_Ball->transform.GetScale().z / 2;
		float ballRight		= m_Ball->transform.GetPosition().z + m_Ball->transform.GetScale().z / 2;
		float ballTop		= m_Ball->transform.GetPosition().x - m_Ball->transform.GetScale().x / 2;
		float ballBottom	= m_Ball->transform.GetPosition().x + m_Ball->transform.GetScale().x / 2;
																					
		float paddleLeft	= paddle->transform.GetPosition().z - paddle->transform.GetScale().z / 2;
		float paddleRight	= paddle->transform.GetPosition().z + paddle->transform.GetScale().z / 2;
		float paddleTop		= paddle->transform.GetPosition().x - paddle->transform.GetScale().x / 2;
		float paddleBottom	= paddle->transform.GetPosition().x + paddle->transform.GetScale().x / 2;

		if (ballLeft >= paddleRight)
		{
			return false;
		}

		if (ballRight <= paddleLeft)
		{
			return false;
		}

		if (ballTop >= paddleBottom)
		{
			return false;
		}

		if (ballBottom <= paddleTop)
		{
			return false;
		}

		return true;
	}

	CollisionType CheckWallCollision() {

		float windowWidth = static_cast<float>(Louron::Engine::Get().GetWindow().GetWidth());
		float windowHeight = static_cast<float>(Louron::Engine::Get().GetWindow().GetHeight());

		// Normalised viewport boundaries
		float screenLeft = -1.0f;
		float screenTop = 1.0f;

		// Transform screen boundaries to world space
		glm::mat4 invProjection = glm::inverse(glm::perspective(glm::radians(60.0f), windowWidth / windowHeight, 0.1f, 100.0f));

		// 3. Transform the coordinates to eye space (Coordinate of Mouse X & Y on View Space)
		glm::vec4 rayEye = invProjection * glm::vec4(screenLeft, screenTop, -1.0f, 1.0f);
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		// 4. Calculate the inverse of the view matrix (View Space -> World Space)
		glm::mat4 invView = glm::inverse(m_SceneCamera.GetViewMatrix());

		// 5. Transform the ray to world coordinates and normalize it (Start Position of Ray in World Space)
		glm::vec4 rayWorld = invView * rayEye;
		rayWorld = glm::normalize(glm::vec4(rayWorld.x, rayWorld.y, rayWorld.z, 0.0f));

		// 6. Calculate the intersection point in world coordinates (Cast Ray and Calculate Position at Custom Intersection on the Y Axis)
		float t = (0.0f - m_SceneCamera.GetPosition().y) / rayWorld.y;
		glm::vec3 boundaryLeftTop = m_SceneCamera.GetPosition() + glm::vec3(rayWorld * t);
		glm::vec3 boundaryRightBottom = -boundaryLeftTop;

		// Define Ball Boundaries
		float ballLeft		= m_Ball->transform.GetPosition().z - m_Ball->transform.GetScale().z / 2;
		float ballRight		= m_Ball->transform.GetPosition().z + m_Ball->transform.GetScale().z / 2;
		float ballTop		= m_Ball->transform.GetPosition().x - m_Ball->transform.GetScale().x / 2;
		float ballBottom	= m_Ball->transform.GetPosition().x + m_Ball->transform.GetScale().x / 2;

		if (ballRight >= boundaryRightBottom.z)
		{
			return CollisionType::Right;
		}

		if (ballLeft <= boundaryLeftTop.z)
		{
			return CollisionType::Left;
		}

		if (ballTop >= boundaryLeftTop.x)
		{
			return CollisionType::Top;
		}

		if (ballBottom <= boundaryRightBottom.x)
		{
			return CollisionType::Bottom;
		}

		return CollisionType::None;
	}

	void ProcessBallMovement() {

		m_Ball->transform.TranslateZ(deltaTime * m_Ball->velocity.x);
		m_Ball->transform.TranslateX(deltaTime * m_Ball->velocity.y);

		m_Light.position = glm::vec3(m_Ball->transform.GetPosition().x, 1.0f, m_Ball->transform.GetPosition().z);
	}

	void ProcessPaddleMovement() {

		// Move Paddle 1 Up and Down
		if (m_Input.GetKey(GLFW_KEY_W)) {
			m_Paddles[0]->transform.TranslateX( deltaTime * m_Paddles[0]->speed);
		}
		if (m_Input.GetKey(GLFW_KEY_S)) {
			m_Paddles[0]->transform.TranslateX( -deltaTime * m_Paddles[0]->speed);
		}

		// Move Paddle 2 Up and Down
		if (m_Input.GetKey(GLFW_KEY_UP)) {
			m_Paddles[1]->transform.TranslateX( deltaTime * m_Paddles[1]->speed);
		}
		if (m_Input.GetKey(GLFW_KEY_DOWN)) {
			m_Paddles[1]->transform.TranslateX( -deltaTime * m_Paddles[1]->speed);
		}
	}

	void ResetGame() {
		m_GameOver = false;
		m_GameRunning = false;
		m_GameStartTimer = 3.0f;

		// Reset Ball, Light, and Paddles
		m_Ball->velocity = { m_Ball->speed, 0.0f };
		m_Ball->transform.SetPosition(glm::vec3(0.0f));
		m_Light.position = glm::vec3(m_Ball->transform.GetPosition().x, 1.0f, m_Ball->transform.GetPosition().z);
		for (auto& paddle : m_Paddles) {
			paddle->transform.SetPositionX(0.0f);
			paddle->score = 0;
		}

	}

	void Draw() override {

		glClearColor(211.0f / 255.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)Louron::Engine::Get().GetWindow().GetWidth() / (float)Louron::Engine::Get().GetWindow().GetHeight(), 0.1f, 100.0f);

		glBindVertexArray(cube_VAO);

		// Render Ball
		if (m_Ball->material.Bind()) {

			m_Ball->material.UpdateUniforms(m_SceneCamera);

			m_Ball->material.GetShader()->SetMat4("model", m_Ball->transform);
			m_Ball->material.GetShader()->SetVec3("u_Light.position", m_Light.position);
			m_Ball->material.GetShader()->SetVec4("u_Light.ambient", m_Light.ambient);
			m_Ball->material.GetShader()->SetVec4("u_Light.diffuse", m_Light.diffuse);
			m_Ball->material.GetShader()->SetVec4("u_Light.specular", m_Light.specular);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			m_Ball->material.UnBind();
		}

		// Render Paddles
		for (const auto& paddle : m_Paddles) {
			if (paddle->material.Bind()) {

				paddle->material.UpdateUniforms(m_SceneCamera);
				paddle->material.GetShader()->SetMat4("model", paddle->transform);

				paddle->material.GetShader()->SetVec3("u_Light.position", m_Light.position);
				paddle->material.GetShader()->SetVec4("u_Light.ambient", m_Light.ambient);
				paddle->material.GetShader()->SetVec4("u_Light.diffuse", m_Light.diffuse);
				paddle->material.GetShader()->SetVec4("u_Light.specular", m_Light.specular);

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