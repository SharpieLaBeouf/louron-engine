#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

using namespace Louron;

class Scene16 : public TestScene {

private:

	std::shared_ptr<Project> m_Project;
	std::shared_ptr<Scene> m_Scene;

	Window& m_Window;
	InputManager& m_Input;

	Entity selected_entity{};

public:

	Scene16() :
		m_Window(Engine::Get().GetWindow()),
		m_Input(Engine::Get().GetInput())
	{
		L_APP_INFO("Loading Scene 16");

	}

	~Scene16() {
		L_APP_INFO("Unloading Scene 16");

	}

	std::shared_ptr<Louron::PBRMaterial> rusted_iron_material = std::make_shared<Louron::PBRMaterial>();

	void OnAttach() override {

		if (!m_Project || !m_Scene) {

			// Load Project and Get Active Scene Handle
			m_Project = Project::LoadProject("Sandbox Project/Sandbox Project.lproj", "PBR Materials.lscene");
			m_Scene = Project::GetActiveScene();

			m_Scene->FindEntityByName("PBR_Rusted_Iron").GetComponent<MeshRenderer>().Materials[0] = rusted_iron_material;

			rusted_iron_material->SetName("Rusted Iron Material");
			rusted_iron_material->SetShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_PBR_Shader"));

			rusted_iron_material->SetAlbedoTexture(Engine::Get().GetTextureLibrary().GetTexture("RustedIron_Albedo"));
			rusted_iron_material->SetMetallicTexture(Engine::Get().GetTextureLibrary().GetTexture("RustedIron_Metallic"));
			rusted_iron_material->SetNormalTexture(Engine::Get().GetTextureLibrary().GetTexture("RustedIron_Normal"));

			for (int row = 0; row < 4; ++row) {
				for (int col = 0; col < 4; ++col) {

					std::shared_ptr<Louron::PBRMaterial> material = std::make_shared<Louron::PBRMaterial>();
					material->SetName("PBR_Sphere_Material_" + std::to_string(row * 4 + col + 1));
					material->SetShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_PBR_Shader"));

					material->SetAlbedoTintColour({ 1.0f, 0.0f, 0.0f, 1.0f });
					material->SetRoughness(glm::clamp((float)col / 4.0f, 0.05f, 1.0f));
					material->SetMetallic((float)row / 4.0f);

					m_Scene->FindEntityByName("PBR_Sphere_" + std::to_string(row * 4 + col + 1)).GetComponent<MeshRenderer>().Materials[0] = material;
				}
			}			
		}
		m_Scene->OnStart();
	}

	void OnDetach() override {

		m_Scene->OnStop();
	}

	float jump_timer = 0.0f;

	void Update() override {

		if (m_Input.GetKey(GLFW_KEY_LEFT_CONTROL) && m_Input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
			auto entity_uuid = std::dynamic_pointer_cast<ForwardPlusPipeline>(m_Scene->GetConfig().ScenePipeline)->PickRenderEntityID(m_Input.GetMousePosition());
			selected_entity = m_Scene->FindEntityByUUID(entity_uuid);
		}

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().Camera->Update((float)Time::Get().GetDeltaTime());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().Camera->GetGlobalPosition());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

		m_Scene->OnUpdate();

		if (m_Input.GetKeyDown(GLFW_KEY_F)) {
			Louron::SpotLightComponent& spotLight = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>();
			spotLight.lightProperties.active = (spotLight.lightProperties.active == GL_TRUE) ? GL_FALSE : GL_TRUE;
		}
	}

	void FixedUpdate() override {
		// No physics to be simulated in this scene.
		//m_Scene->OnFixedUpdate();
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

		if (ImGui::TreeNode("PBR Material: %s", rusted_iron_material->GetName().c_str())) {

			ImGui::Text("Albedo Texture: %s", rusted_iron_material->IsAlbedoTextureSet() ? rusted_iron_material->GetAlbedoTexture()->GetName().c_str() : "None");
			ImGui::Text("Metallic Texture: %s", rusted_iron_material->IsMetallicTextureSet() ? rusted_iron_material->GetMetallicTexture()->GetName().c_str() : "None");
			ImGui::Text("Normal Texture: %s", rusted_iron_material->IsNormalTextureSet() ? rusted_iron_material->GetNormalTexture()->GetName().c_str() : "None");

			ImGui::Separator();

			glm::vec4 colour = rusted_iron_material->GetAlbedoTintColour();
			ImGui::ColorPicker4("Albedo Colour", glm::value_ptr(colour));
			rusted_iron_material->SetAlbedoTintColour(colour);

			if(!rusted_iron_material->IsMetallicTextureSet()) {
				float metallic = rusted_iron_material->GetMetallic();
				ImGui::DragFloat("Metallic", &metallic, 0.05f, 0.0f, 1.0f, "%.2f");
				rusted_iron_material->SetMetallic(metallic);
			}

			float roughness = rusted_iron_material->GetRoughness();
			ImGui::DragFloat("Roughness", &roughness, 0.05f, 0.0f, 1.0f, "%.2f");
			rusted_iron_material->SetRoughness(roughness);

			ImGui::TreePop();
		}

		if (selected_entity && ImGui::TreeNode("Entity Transform")) {
			ImGui::Text("Entity Name: %s", selected_entity.GetName().c_str());
			DisplayTransformGUI(selected_entity);

			ImGui::TreePop();
		}

		if (ImGui::Button("Toggle Camera Movement")) m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().Camera->ToggleMovement();

		if (ImGui::Button("Reset Scene")) {

			m_Scene->OnStop();

			m_Scene = m_Project->LoadScene("PBR Materials.lscene");
			m_Project->SetActiveScene(m_Scene);

			m_Scene->FindEntityByName("PBR_Rusted_Iron").GetComponent<MeshRenderer>().Materials[0] = rusted_iron_material;

			rusted_iron_material->SetName("Rusted Iron Material");
			rusted_iron_material->SetShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_PBR_Shader"));

			rusted_iron_material->SetAlbedoTexture(Engine::Get().GetTextureLibrary().GetTexture("RustedIron_Albedo"));
			rusted_iron_material->SetMetallicTexture(Engine::Get().GetTextureLibrary().GetTexture("RustedIron_Metallic"));
			rusted_iron_material->SetNormalTexture(Engine::Get().GetTextureLibrary().GetTexture("RustedIron_Normal"));

			for (int row = 0; row < 4; ++row) {
				for (int col = 0; col < 4; ++col) {

					std::shared_ptr<Louron::PBRMaterial> material = std::make_shared<Louron::PBRMaterial>();
					material->SetName("PBR_Sphere_Material_" + std::to_string(row * 4 + col + 1));
					material->SetShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_PBR_Shader"));

					material->SetAlbedoTintColour({ 1.0f, 0.0f, 0.0f, 1.0f });
					material->SetRoughness(glm::clamp((float)col / 4.0f, 0.05f, 1.0f));
					material->SetMetallic((float)row / 4.0f);

					m_Scene->FindEntityByName("PBR_Sphere_" + std::to_string(row * 4 + col + 1)).GetComponent<MeshRenderer>().Materials[0] = material;
				}
			}

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