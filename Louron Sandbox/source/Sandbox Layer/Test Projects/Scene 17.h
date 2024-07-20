#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include <glm/gtc/noise.hpp>

using namespace Louron;

class Scene17 : public TestScene {

private:

	std::shared_ptr<Project> m_Project;
	std::shared_ptr<Scene> m_Scene;

	Window& m_Window;
	InputManager& m_Input;

	Entity selected_entity{};
	AssetHandle texture_handle = NULL_UUID;
	AssetHandle sb_material_handle = NULL_UUID;

	std::shared_ptr<EditorAssetManager> m_AssetManager;

	Entity candle_entity;

	std::shared_ptr<Prefab> torch_prefab = nullptr;

public:

	Scene17() :
		m_Window(Engine::Get().GetWindow()),
		m_Input(Engine::Get().GetInput())
	{
		L_APP_INFO("Loading Scene 17");

	}

	~Scene17() {
		L_APP_INFO("Unloading Scene 17");

	}

	void OnAttach() override {

		std::srand(static_cast<unsigned int>(Time::Get().GetCurrTime()));

		// Load Project and Get Active Scene Handle
		if (m_Project)
			Louron::Project::SetActiveProject(m_Project);
		else
			m_Project = Project::LoadProject("Sandbox Project/Sandbox Project.lproj");

		if (!m_Scene) {

			m_Scene = Project::GetActiveScene();

			candle_entity = m_Scene->FindEntityByName("Candle");
						
			m_AssetManager = std::make_shared<EditorAssetManager>();
			m_AssetManager->ImportAsset("Sponza/Sponza.obj");

			torch_prefab = std::make_shared<Prefab>(m_Scene->FindEntityByName("Light Source 0"));
			for(int i = 0; i < 25; i++) {
				std::string str = "Light Source " + std::to_string(i);
				m_Scene->DestroyEntity(m_Scene->FindEntityByName(str));
			}

		}
		m_Scene->OnStart();
	}

	void OnDetach() override {

		m_Scene->OnStop();
	}

	float minIntensity = 4.5f;
	float maxIntensity = 6.0f;
	float flickerSpeed = 3.0f;

	float time = 0.0f;

	std::unordered_map<std::string, float> light_bob_offset;

	void Update() override {

		// Apply a bobbing effect to all PointLights in scene that are not the Candle
		const float amplitude = 0.005f; // Maximum height change
		const float frequency = 1.0f; // Speed of bobbing (frequency in Hz)
		auto view = m_Scene->GetAllEntitiesWith<TagComponent, Transform, PointLightComponent>();
		if (view.begin() != view.end()) {
			for (const auto& entity : view) {
				
				auto [tag, transform, light] = view.get<TagComponent, Transform, PointLightComponent>(entity);

				if (tag.Tag == "Candle")
					continue;

				if(light_bob_offset.find(tag.Tag) == light_bob_offset.end())
					light_bob_offset[tag.Tag] = glm::linearRand(-1.0f, 1.0f);

				// Calculate the new Y position using a sine function for bobbing effect
				float bobbingOffset = amplitude * std::sin(frequency * static_cast<float>(Time::Get().GetCurrTime()) + light_bob_offset[tag.Tag]);

				// Set the new Y position
				transform.TranslateY(bobbingOffset);

			}
		}

		// Flicker Candle Light Intensity
		time += Time::GetDeltaTime() * flickerSpeed;
		float noise = glm::perlin(glm::vec2{ time, 0.0f });
		if (candle_entity) candle_entity.GetComponent<PointLightComponent>().lightProperties.intensity = static_cast<GLfloat>(std::lerp(minIntensity, maxIntensity, noise));

		// Mouse Pick Entity
		if (m_Input.GetKey(GLFW_KEY_LEFT_CONTROL) && m_Input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
			auto entity_uuid = std::dynamic_pointer_cast<ForwardPlusPipeline>(m_Scene->GetConfig().ScenePipeline)->PickRenderEntityID(m_Input.GetMousePosition());
			selected_entity = m_Scene->FindEntityByUUID(entity_uuid);
		}

		// Instantiate Torch Prefab!
		if (m_Input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {


			// Get the camera transform and direction
			auto cameraEntity = m_Scene->FindEntityByName("Main Camera");
			auto cameraTransform = cameraEntity.GetComponent<Transform>();
			auto cameraDirection = cameraEntity.GetComponent<Louron::CameraComponent>().CameraInstance->GetCameraDirection();

			//Instantiate the bullet prefab and set its position
			auto entity = m_Scene->InstantiatePrefab(torch_prefab, cameraTransform);

			glm::vec3 position = cameraTransform.GetGlobalPosition() + (cameraDirection * 10.0f);
			entity.GetComponent<Transform>().SetPosition(position);
			entity.GetComponent<PointLightComponent>().ambient = { 1, 0.5, 0.200000003, 1.0f };
			entity.GetComponent<PointLightComponent>().diffuse = { 1, 0.5, 0.200000003, 1.0f };
			entity.GetComponent<PointLightComponent>().specular = { 1, 0.5, 0.200000003, 1.0f };
		}

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().CameraInstance->Update(Time::Get().GetDeltaTime());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().CameraInstance->GetGlobalPosition());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().CameraInstance->GetCameraDirection(), 1.0f);

		// Toggle Flash Light
		if (m_Input.GetKeyDown(GLFW_KEY_F)) {
			Louron::SpotLightComponent& spotLight = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>();
			spotLight.lightProperties.active = (spotLight.lightProperties.active == GL_TRUE) ? GL_FALSE : GL_TRUE;
		}

		// Call Scene Update
		m_Scene->OnUpdate();
	}

	void FixedUpdate() override {

		m_Scene->OnFixedUpdate();
	}

	void UpdateGUI() override {

		m_Scene->OnUpdateGUI();

		ImGui::Begin("Application Scene Control", (bool*)0,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse
		);

		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

		ImGui::Separator();

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

		if (ImGui::TreeNode("Profiling")) {

			for (auto& result : Profiler::Get().GetResults()) {

				char label[128];
				strcpy_s(label, result.second.Name);
				strcat_s(label, " %.3fms");

				ImGui::Text(label, result.second.Time);
			}

			ImGui::TreePop();
		}

		if (selected_entity && ImGui::TreeNode("Entity Transform")) {
			ImGui::Text("Entity Name: %s", selected_entity.GetName().c_str());
			DisplayTransformGUI(selected_entity);

			ImGui::TreePop();
		}

		if (ImGui::Button("Toggle Camera Movement")) m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->ToggleMovement();

		if (ImGui::Button("Reset Scene")) {

			m_Scene->OnStop();

			m_Scene = m_Project->LoadScene("Sandbox Project Scene.lscene");
			m_Project->SetActiveScene(m_Scene);

			candle_entity = m_Scene->FindEntityByName("Candle");

			m_Scene->OnStart();
		}

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

		ImGui::End();

	}

	void DisplayTransformGUI(Entity entity) {
		auto& trans = entity.GetComponent<Transform>();

		glm::vec3 temp = trans.GetLocalPosition();
		if (ImGui::DragFloat3("Local Position", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f"))
			trans.SetPosition(temp);

		temp = trans.GetLocalRotation();
		if (ImGui::DragFloat3("Local Rotation", glm::value_ptr(temp), 1.0f, 0, 0, "%.2f"))
			trans.SetRotation(temp);

		temp = trans.GetLocalScale();
		if (ImGui::DragFloat3("Local Scale", glm::value_ptr(temp), 0.01f, 0, 0, "%.2f"))
			trans.SetScale(temp);
	}
};