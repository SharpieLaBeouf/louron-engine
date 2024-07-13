#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include "Scene/Scene Serializer.h"

#include "glm/gtx/string_cast.hpp"
#include <bitset>
#include <cmath>

using namespace Louron;

constexpr float STAMINA_MAX = 1.0f;
constexpr float STAMINA_RECHARGE_MODIFIER = 0.5f; // This is multiplied by the deltaTime so the recharging can be slower or quicker per frame
constexpr float DEAD_ZONE_FLOOR = -20.0f;
constexpr float MAP_CHANGE_TIMER_CONSTANT = 5.0f;

// KNERPIX! Somewhat of a version of my year 12 software major work :D fun to revisit
class Scene14 : public TestScene {

private:

	std::shared_ptr<Project> m_Project;
	std::shared_ptr<Scene> m_Scene;

	Window& m_Window;
	InputManager& m_Input;

public:
	Scene14() : m_Window(Engine::Get().GetWindow()), m_Input(Engine::Get().GetInput()) { }
	~Scene14() { L_APP_INFO("Unloading Scene 14 - KNERPIX D:"); }

	void OnAttach() override {

		std::srand((unsigned int)std::time(0));

		if (!m_Project || !m_Scene) {
			L_APP_INFO("Loading Scene 14 - KNERPIX :D");

			// Load Project and Get Active Scene Handle
			m_Project = Project::LoadProject("Sandbox Project/Sandbox Project.lproj", "Knerpix.lscene");
			m_Scene = Project::GetActiveScene();

			ball[0].ball = m_Scene->FindEntityByName("Ball1");
			ball[1].ball = m_Scene->FindEntityByName("Ball2");

			ball[0].starting_position = ball[0].ball.GetComponent<Transform>().GetGlobalPosition();
			ball[1].starting_position = ball[1].ball.GetComponent<Transform>().GetGlobalPosition();

			auto customCollisionCallback = [&](Entity& self, Entity& other) -> void {

				if ((self.GetName() == "Ball1" && other.GetName() == "Ball2") || (self.GetName() == "Ball2" && other.GetName() == "Ball1")) {

					if (!self.HasComponent<Rigidbody>() || !other.HasComponent<Rigidbody>())
						return;

					PxVec3 self_velocity = self.GetComponent<Rigidbody>().GetActor()->GetLinearVelocity();
					PxVec3 other_velocity = other.GetComponent<Rigidbody>().GetActor()->GetLinearVelocity();

					if (self_velocity.magnitude() <= other_velocity.magnitude()) {

						// Calculate the direction from A to B
						PxVec3 direction = self.GetComponent<Rigidbody>().GetActor()->GetGlobalPose().p - other.GetComponent<Rigidbody>().GetActor()->GetGlobalPose().p;
						direction.normalize();

						// Calculate impulse magnitude (adjust according to your game's requirements)
						float impulseMagnitude = 600.0f;

						self.GetComponent<Rigidbody>().ApplyForce(glm::vec3( direction.x, direction.y, direction.z ) * impulseMagnitude, PxForceMode::eFORCE);

						Audio::Get().PlayAudioFile("assets/Audio/impact.mp3");

					}

				}

			};

			ball[0].ball.GetComponent<Louron::SphereCollider>().SetOnCollideCallback(customCollisionCallback);
			ball[1].ball.GetComponent<Louron::SphereCollider>().SetOnCollideCallback(customCollisionCallback);

		}
		m_Scene->OnStart();
	}

	void OnDetach() override {
		m_Scene->OnStop();
	}

	void ResetGame() {

		game_properties.game_over = false;
		game_properties.lerp_timer = 0.0f;
		game_properties.lerp_start_pos = {};

		ResetEnvironment();

		for (int i = 0; i < ball.size(); i++) {

			ball[i].ball.GetComponent<Rigidbody>().SetKinematic(false);
			ball[i].lives = 3;
			ball[i].winner = false;
		}
	}

	struct Ball {

		Entity ball;

		int lives = 3;
		bool winner = false;
		float speed = 10.0f;

		float stamina = STAMINA_MAX;
		bool stamina_exhausted = false;

		glm::vec3 starting_position{};

		bool IsAlive() const { return lives > 0 ? true : false;  }

	};

	struct GameProperties {

		float lerp_timer = 0.0f;
		glm::vec3 lerp_start_pos = {};

		bool game_over = false;
		bool app_control_ui = false;

		uint16_t current_map = 0;
		bool updating_map = false;
		float map_change_timer = MAP_CHANGE_TIMER_CONSTANT;

		void ChangeMap(const uint16_t& new_map) {
			updating_map = true;
			current_map = new_map;
		}

	} game_properties;

	std::array<Ball, 2> ball{};

	void Update() override {

		if (!game_properties.game_over) {

			UpdateMap();

			ProcessStamina();
			CheckForDeaths();

			ProcessWinnerAnimation();
			
		}

		// Test Mouse Picking Functionality
		if (m_Input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {

			if (auto render_pipeline = std::dynamic_pointer_cast<Louron::ForwardPlusPipeline>(m_Scene->GetConfig().ScenePipeline); render_pipeline) {

				if (auto entity = m_Scene->FindEntityByUUID(render_pipeline->PickRenderEntityID(m_Input.GetMousePosition())); entity) {
					L_APP_INFO("You Have Clicked On Entity: {0}", entity.GetName().c_str());
				}
				else {
					L_APP_WARN("No Entity Found On Mouse Position.");
				}

			}

		}

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().Camera->Update((float)Time::Get().GetDeltaTime());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().Camera->GetGlobalPosition());

		m_Scene->OnUpdate();
	}

	void FixedUpdate() override {

		ProcessMovement();

		m_Scene->OnFixedUpdate();
	}

	void UpdateGUI() override {

		m_Scene->OnUpdateGUI();

		// If we are officially finished the game, show GameOver GUI
		if(game_properties.game_over)
			ProcessGameOverGUI();
		
		// If both balls are in action, process Gameplay HUD
		if(ball[0].IsAlive() && ball[1].IsAlive())
			ProcessGameplayHUD();
		
		// Toggle App Control UI
		if ((m_Input.GetKey(GLFW_KEY_LEFT_CONTROL) && m_Input.GetKeyDown(GLFW_KEY_F11)) || (m_Input.GetKey(GLFW_KEY_RIGHT_CONTROL) && m_Input.GetKeyDown(GLFW_KEY_F11)))
			game_properties.app_control_ui = !game_properties.app_control_ui;

		// Display App Control UI
		if(game_properties.app_control_ui)
			ProcessAppControlGUI();
	}

	void ProcessAppControlGUI() {

		if(ImGui::Begin("Application Scene Control", (bool*)0, NULL))
		{
			if (ImGui::TreeNode("Project and Scene Menu")) {

				std::string pipeline;
				switch (m_Scene->GetConfig().ScenePipelineType) {
				case L_RENDER_PIPELINE::FORWARD:
					pipeline = "Forward";
					break;
				case L_RENDER_PIPELINE::FORWARD_PLUS:
					pipeline = "Forward Plus";
					break;
				case L_RENDER_PIPELINE::DEFERRED:
					pipeline = "Deferred";
					break;
				}

				ImGui::Text(("Project Loaded: " + m_Project->GetConfig().Name).c_str());
				ImGui::Text(("Scene Loaded: " + m_Scene->GetConfig().Name).c_str());

				ImGui::Separator();

				ImGui::Text(("Asset Directory: " + m_Scene->GetConfig().AssetDirectory.string()).c_str());
				ImGui::Text(("Rendering Technique: " + pipeline).c_str());

				ImGui::Separator();

				if (ImGui::Button("Save Project")) m_Project->SaveProject();
				if (ImGui::Button("Save Scene")) m_Project->SaveScene();


				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Profiling")) {

				for (auto& result : Profiler::Get().GetResults()) {

					char label[128];
					strcpy_s(label, result.second.Name);
					strcat_s(label, " %.3fms");

					ImGui::Text(label, result.second.Time);
				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Scene Time")) {

				float timeScale = Time::GetTimeScale();

				ImGui::Text("Time Scale      : %.6f", timeScale);
				ImGui::Text("Unscaled DT	 : %.6f", Time::GetUnscaledDeltaTime());
				ImGui::Text("Delta Time      : %.6f", Time::GetDeltaTime());
				ImGui::Text("Unscaled FDT    : %.6f", Time::GetUnscaledFixedDeltaTime());
				ImGui::Text("Fixed DT        : %.6f", Time::GetFixedDeltaTime());
				ImGui::Text("Fixed Update Hz : %.6f", (float)Time::GetFixedUpdatesHz());

				if (ImGui::DragFloat("Time Scale", &timeScale, 0.1f, 0.1f, 2.0f)) { Time::SetTimeScale(timeScale); }

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Ball 1")) {

				auto ball_ref = ball[0].ball;
				if (ball_ref) {
					DisplayTransformGUI(ball_ref);
				}

				if (ImGui::Button("Delete Ball 1")) {
					m_Scene->DestroyEntity(ball_ref);
					ball[1].ball = {};
				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Ball 2")) {

				auto ball_ref = ball[1].ball;
				if (ball_ref) {
					DisplayTransformGUI(ball_ref);
				}

				if (ImGui::Button("Delete Ball 2")) {
					m_Scene->DestroyEntity(ball_ref);
					ball[1].ball = {};
				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Wall")) {

				static int wall_selection = 0;
				ImGui::DragInt("Wall Selection", &wall_selection, 1.0f, 0, 15);

				auto wall = m_Scene->FindEntityByName("Wall " + std::to_string(wall_selection));
				if (wall) {

					DisplayTransformGUI(wall);

					if (ImGui::Button(std::string("Delete Wall " + std::to_string(wall_selection)).c_str())) {
						m_Scene->DestroyEntity(wall);
					}
				}
				else {
					ImGui::Text("Wall Entity No Longer Exists!");
				}


				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Floor")) {

				auto floor = m_Scene->FindEntityByName("Floor");
				if (floor) {

					DisplayTransformGUI(floor);

					if (ImGui::Button("Delete Floor")) {
						m_Scene->DestroyEntity(floor);
					}
				}
				else {
					ImGui::Text("Floor Entity No Longer Exists!");
				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Other Scene Settings")) {

				if (ImGui::Button("Toggle Camera Movement")) {
					m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().Camera->ToggleMovement();
				}

				if (ImGui::Button("Reset Scene")) {
					ResetGame();
				}

				ImGui::TreePop();
			}

			ImGui::End();
		}
	}

	void DisplayTransformGUI(Entity entity) {
		auto& trans = entity.GetComponent<Transform>();

		glm::vec3 temp = trans.GetLocalPosition();
		if (ImGui::DragFloat3("Position", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f"))
			trans.SetPosition(temp);

		temp = trans.GetLocalRotation();
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(temp), 1.0f, 0, 0, "%.2f"))
			trans.SetRotation(temp);

		temp = trans.GetLocalScale();
		if (ImGui::DragFloat3("Scale", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f"))
			trans.SetScale(temp);
	}

	void ProcessGameplayHUD() {

		// Stamina Bars 
		ImGuiWindowFlags window_flags = 
			ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse | 
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings | 
			ImGuiWindowFlags_NoInputs | 
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoDecoration;

		ImVec2 bar_pos(10.0f, (float)m_Window.GetHeight() - 60.0f);
		ImVec2 bar_size((float)m_Window.GetWidth() * 0.3f, 20.0f);

		ImGui::Begin("Stamina1", NULL, window_flags);

		ImGui::SetWindowPos(ImVec2(bar_pos.x, bar_pos.y));
		ImGui::SetWindowSize(ImVec2(bar_size.x, bar_size.y));

		// Draw the stamina bar
		ImVec4 stamina_bar_colour = ball[0].stamina_exhausted ? ImVec4(1.0f - (ball[0].stamina / STAMINA_MAX), (ball[0].stamina / STAMINA_MAX), 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, stamina_bar_colour);
		ImGui::ProgressBar(ball[0].stamina / STAMINA_MAX, ImVec2(-1, 0), " ");
		ImGui::PopStyleColor();

		ImGui::End();

		bar_pos = ImVec2((float)m_Window.GetWidth() - ((float)m_Window.GetWidth() * 0.3f) - 10.0f, (float)m_Window.GetHeight() - 60.0f);  // Position at (window_width - 30% width - 10, window_height - 60)
		bar_size = ImVec2((float)m_Window.GetWidth() * 0.3f, 20.0f);

		ImGui::Begin("Stamina2", NULL, window_flags);

		ImGui::SetWindowPos(ImVec2(bar_pos.x, bar_pos.y));
		ImGui::SetWindowSize(ImVec2(bar_size.x, bar_size.y));

		// Draw the stamina bar
		stamina_bar_colour = ball[1].stamina_exhausted ? ImVec4(1.0f - (ball[1].stamina / STAMINA_MAX), (ball[1].stamina / STAMINA_MAX), 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, stamina_bar_colour);
		ImGui::ProgressBar(ball[1].stamina / STAMINA_MAX, ImVec2(-1, 0), " ");
		ImGui::PopStyleColor();

		ImGui::End();


		ImGui::Begin("Lives1", NULL, window_flags);
		ImGui::SetWindowFontScale(2.0f);

		std::string lives_string = std::to_string(ball[0].lives) + "/3 ";

		bar_size = ImGui::CalcTextSize(lives_string.c_str());
		bar_pos = ImVec2(40.0f, 40.0f);

		ImGui::SetWindowPos(ImVec2(bar_pos.x, bar_pos.y));
		ImGui::SetWindowSize(ImVec2(bar_size.x, bar_size.y));

		ImGui::Text("%s", lives_string.c_str());

		ImGui::End();


		ImGui::Begin("Lives2", NULL, window_flags);
		ImGui::SetWindowFontScale(2.0f);

		lives_string = std::to_string(ball[1].lives) + "/3 ";

		bar_size = ImGui::CalcTextSize(lives_string.c_str());
		bar_pos = ImVec2((float)m_Window.GetWidth() - bar_size.x - 40.0f, 40.0f);

		ImGui::SetWindowPos(ImVec2(bar_pos.x, bar_pos.y));
		ImGui::SetWindowSize(ImVec2(bar_size.x, bar_size.y));

		ImGui::Text("%s", lives_string.c_str());

		ImGui::End();

	}

	void ProcessGameOverGUI() {

		ImGui::Begin("Game Over!", (bool*)0,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse
		);

		glm::vec2 size = { (float)m_Window.GetWidth() / 6.0f, (float)m_Window.GetHeight() / 6.0f };

		ImGui::SetWindowPos(ImVec2( (float)m_Window.GetWidth() / 2.0f - size.x / 2.0f, (float)m_Window.GetHeight() / 2.0f - size.y / 2.0f));
		ImGui::SetWindowSize(ImVec2(size.x, size.y));

		if (ball[0].winner) ImGui::Text("Player 1 Wins!");
		if (ball[1].winner) ImGui::Text("Player 2 Wins!");
		
		if (ImGui::Button("Reset Game?"))
			ResetGame();

		ImGui::End();

	}

	void UpdateMap() {

		if (!ball[0].IsAlive() || !ball[1].IsAlive())
			return;

		if (!game_properties.updating_map) {

			game_properties.map_change_timer -= Time::GetDeltaTime();

			if (game_properties.map_change_timer <= 0.0f) {
				game_properties.ChangeMap(glm::linearRand(0, 65535));
			}
			return;
		}

		// Map is changing, lets move things around!
		game_properties.map_change_timer += Time::GetDeltaTime() * 1.5f;

		if (game_properties.map_change_timer >= MAP_CHANGE_TIMER_CONSTANT) {
			// Map Changing Finished!
			game_properties.updating_map = false;
		}

		float translationIncrement = Time::GetDeltaTime() * (50.0f / MAP_CHANGE_TIMER_CONSTANT) * 1.5f;
		for (int i = 0; i < 16; i++) {

			if ((game_properties.current_map & (1U << i)) != 0) {

				auto wall = m_Scene->FindEntityByName("Wall " + std::to_string(i));
				if (wall && wall.GetComponent<Transform>().GetGlobalPosition().y < 51.85f) {

					wall.GetComponent<Transform>().TranslateY(translationIncrement);
				}

			}
			else {

				auto wall = m_Scene->FindEntityByName("Wall " + std::to_string(i));
				if (wall && wall.GetComponent<Transform>().GetGlobalPosition().y > 1.85f) {

					wall.GetComponent<Transform>().TranslateY(-translationIncrement);
				}
			}

		}		
	}

	void ProcessWinnerAnimation() {

		for (auto& ball_ref : ball) {
			if (ball_ref.winner)
				if (game_properties.lerp_timer < 1.5f) {

					game_properties.lerp_timer += Time::GetDeltaTime();
					float t = game_properties.lerp_timer / 1.5f;

					glm::vec3 end_pos = glm::vec3(-23.0f, 28.0f, 2.0f);

					glm::vec3 interpolatedPos = glm::mix(game_properties.lerp_start_pos, end_pos, t);

					ball_ref.ball.GetComponent<Transform>().SetPosition(interpolatedPos);
				}
				else game_properties.game_over = true;
		}
	}

	void ResetEnvironment() {

		// Reset Balls
		for (int i = 0; i < ball.size(); i++) {

			ball[i].stamina = STAMINA_MAX;
			ball[i].stamina_exhausted = false;

			ball[i].ball.GetComponent<Transform>().SetPosition(ball[i].starting_position);
			ball[i].ball.GetComponent<Transform>().SetRotation(glm::vec3(0.0f));

			ball[i].ball.GetComponent<Rigidbody>().GetActor()->SetLinearVelocity(glm::vec3(0.0f));
			ball[i].ball.GetComponent<Rigidbody>().GetActor()->SetAngularVelocity(PxVec3(0.0f));
		}


		// Reset Map
		game_properties.current_map = 0;
		game_properties.map_change_timer = MAP_CHANGE_TIMER_CONSTANT;
		game_properties.updating_map = false;

		for (int i = 0; i < 16; i++) {
			auto wall = m_Scene->FindEntityByName("Wall " + std::to_string(i));
			if (wall) {
				wall.GetComponent<Transform>().SetPositionY(1.85f);
			}
		}
	}

	void CheckForDeaths() {

		if (!ball[0].IsAlive() || !ball[1].IsAlive()) {

			// One of the players are like proper dead so return
			return;
		}

		std::array<bool, 2> dead_ball
		{ 
			(ball[0].ball.GetComponent<Transform>().GetGlobalPosition().y <= DEAD_ZONE_FLOOR), 
			(ball[1].ball.GetComponent<Transform>().GetGlobalPosition().y <= DEAD_ZONE_FLOOR) 
		};

		if (!dead_ball[0] && !dead_ball[1]) {

			// Both players IN ACTION!
			return;
		}

		ResetEnvironment();

		for (int i = 0; i < ball.size(); i++) {

			// If this ball isn't dead, must be next ball
			if (!dead_ball[i])
				continue;

			ball[i].lives -= 1;

			if (ball[i].lives <= 0) {

				// Flag winner and set rigidbody to kinematic
				int winner_id = (i == 0) ? 1 : 0;
				ball[winner_id].winner = true;
				game_properties.lerp_start_pos = ball[winner_id].ball.GetComponent<Transform>().GetGlobalPosition();
				ball[winner_id].ball.GetComponent<Rigidbody>().SetKinematic(true);

				// Change loser position and doom to everlasting falling in the world muahah
				ball[i].ball.GetComponent<Transform>().SetPosition({ -200.0f, -20.0f, 0.0f });
				break;
			}

			L_APP_INFO("Ball {0} Lives Left: {1}", std::to_string(i + 1), ball[i].lives);
		}
		
	}

	void ProcessStamina() {

		for (int i = 0; i < ball.size(); i++) {

			int GLFW_KEY = (i == 0) ? GLFW_KEY_LEFT_SHIFT : GLFW_KEY_RIGHT_SHIFT;

			if (m_Input.GetKey(GLFW_KEY) && !ball[i].stamina_exhausted)
			{
				ball[i].stamina -= Time::GetDeltaTime();
				ball[i].speed = 25.0f;

				if (ball[i].stamina < 0.0f) ball[i].stamina_exhausted = true;
			}
			else {

				ball[i].speed = 10.0f;
				if (ball[i].stamina < STAMINA_MAX) {
					ball[i].stamina += Time::GetDeltaTime() * STAMINA_RECHARGE_MODIFIER;
				}
				else if (ball[i].stamina > STAMINA_MAX) {
					ball[i].stamina = STAMINA_MAX;
					ball[i].stamina_exhausted = false;
				}
			}
		}
	}

	void ProcessMovement() {

		if (game_properties.game_over)
			return;

		if (!m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().Camera->IsMovementEnabled()) {

			//ROLL THE SPHERE!
			if (m_Input.GetKey(GLFW_KEY_W)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(1.0f, 0.0f, 0.0f) * ball[0].speed);
			}
			if (m_Input.GetKey(GLFW_KEY_S)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(-1.0f, 0.0f, 0.0f) * ball[0].speed);
			}
			if (m_Input.GetKey(GLFW_KEY_A)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, -1.0f) * ball[0].speed);
			}
			if (m_Input.GetKey(GLFW_KEY_D)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, 1.0f) * ball[0].speed);
			}

			//ROLL THE SPHERE!
			if (m_Input.GetKey(GLFW_KEY_UP)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(1.0f, 0.0f, 0.0f) * ball[1].speed);
			}
			if (m_Input.GetKey(GLFW_KEY_DOWN)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(-1.0f, 0.0f, 0.0f) * ball[1].speed);
			}
			if (m_Input.GetKey(GLFW_KEY_LEFT)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, -1.0f) * ball[1].speed);
			}
			if (m_Input.GetKey(GLFW_KEY_RIGHT)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, 1.0f) * ball[1].speed);
			}
		}
	}

};