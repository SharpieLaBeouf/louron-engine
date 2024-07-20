#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include "Scene/Scene Serializer.h"

#include "glm/gtc/random.hpp"
#include "glm/gtx/string_cast.hpp"
#include <bitset>

using namespace Louron;

class Scene13 : public TestScene {

private:

	std::shared_ptr<Project> m_Project;
	std::shared_ptr<Scene> m_Scene;

	Window& m_Window;
	InputManager& m_Input;
	ShaderLibrary& m_ShaderLib;
	TextureLibrary& m_TextureLib;

public:
	Scene13() :
		m_Window(Engine::Get().GetWindow()),
		m_Input(Engine::Get().GetInput()),
		m_ShaderLib(Engine::Get().GetShaderLibrary()),
		m_TextureLib(Engine::Get().GetTextureLibrary())
	{
		L_APP_INFO("Loading Scene 13");

	}

	~Scene13() {
		L_APP_INFO("Unloading Scene 13");

	}

	void OnAttach() override {

		// Load Project and Get Active Scene Handle
		if (m_Project)
			Louron::Project::SetActiveProject(m_Project);
		else
			m_Project = Project::LoadProject("Sandbox Project/Sandbox Project.lproj", "Physics Scene.lscene");

		if (!m_Scene)
			m_Scene = m_Project->GetActiveScene();

		m_Scene->OnStart();
	}

	void OnDetach() override {

		m_Scene->OnStop();
	}

	float jump_timer = 0.0f;

	void Update() override {

		// Small Jump!
		if (m_Input.GetKeyDown(GLFW_KEY_E) && jump_timer <= 0.0f) {
			m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 400.0f, 0.0f));
			jump_timer = 0.8f;
		} 
		else if (jump_timer > 0.0f) {
			jump_timer -= (float)Time::Get().GetDeltaTime();
		}

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().CameraInstance->Update((float)Time::Get().GetDeltaTime());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().CameraInstance->GetGlobalPosition());

		m_Scene->OnUpdate();
	}

	void FixedUpdate() override {

		// Update MeshRenderer of Objects if Sleeping or Not
		{
			auto view = m_Scene->GetAllEntitiesWith<Rigidbody, MeshRenderer, BoxCollider>();
			for (const auto& entity_ID : view) {

				auto [rigidbody, mesh_renderer] = view.get<Rigidbody, MeshRenderer>(entity_ID);

				if (rigidbody.IsKinematicEnabled())
					continue;

				if (rigidbody.GetActor() && rigidbody.GetActor()->GetActor()->isSleeping()) {
					mesh_renderer.LinkMeshRenderer(m_Scene->GetResources()->GetMeshRenderer("Red_Cube"));
				}
				else {
					mesh_renderer.LinkMeshRenderer(m_Scene->GetResources()->GetMeshRenderer("Green_Cube"));
				}
			}
			auto view2 = m_Scene->GetAllEntitiesWith<Rigidbody, MeshRenderer, SphereCollider>();
			for (const auto& entity_ID : view2) {

				auto [rigidbody, mesh_renderer] = view2.get<Rigidbody, MeshRenderer>(entity_ID);

				if (rigidbody.entity->GetName() == "Ball1")
					continue;

				if (rigidbody.IsKinematicEnabled())
					continue;

				if (rigidbody.GetActor() && rigidbody.GetActor()->GetActor()->isSleeping()) {
					mesh_renderer.LinkMeshRenderer(m_Scene->GetResources()->GetMeshRenderer("Red_Cube"));
				}
				else {
					mesh_renderer.LinkMeshRenderer(m_Scene->GetResources()->GetMeshRenderer("Green_Cube"));
				}
			}
		}

		if(!m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->IsMovementEnabled()) {

			float multiplier = 10.0f;

			if (m_Input.GetKey(GLFW_KEY_LEFT_SHIFT))
				multiplier *= 5.0f;

			//ROLL THE SPHERE!
			if (m_Input.GetKey(GLFW_KEY_W)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(1.0f, 0.0f, 0.0f) * multiplier);
			}
			if (m_Input.GetKey(GLFW_KEY_S)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(-1.0f, 0.0f, 0.0f) * multiplier);
			}
			if (m_Input.GetKey(GLFW_KEY_A)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, -1.0f) * multiplier);
			}
			if (m_Input.GetKey(GLFW_KEY_D)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, 1.0f) * multiplier);
			}
		}

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
		ImGui::SetWindowSize(ImVec2((float)m_Window.GetWidth() / 6.0f, (float)m_Window.GetHeight()));

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
		
		if (ImGui::Button("Toggle Camera Movement")) m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().CameraInstance->ToggleMovement();
		
		if (ImGui::Button("Reset Physics Cubes")) {

			auto view = m_Scene->GetAllEntitiesWith<Transform, Rigidbody, BoxCollider>();
			for (const auto& entity_ID : view) {
								
				auto [transform, rigidbody] = view.get<Transform, Rigidbody>(entity_ID);

				if (rigidbody.IsKinematicEnabled())
					continue;
				
				transform.SetPosition({ glm::linearRand(-20.0f, 20.0f), glm::linearRand(10.0f, 40.0f), glm::linearRand(-20.0f, 20.0f) });
				transform.SetRotation({ glm::linearRand(0.00f, 360.0f), glm::linearRand(0.0f, 360.0f), glm::linearRand(0.00f, 360.0f) });
				transform.SetScale   ({ glm::linearRand(0.5f, 2.0f),    glm::linearRand(0.5f, 2.0f),   glm::linearRand(0.5f, 2.0f)    });

				rigidbody.ApplyForce({ glm::linearRand(-400.0f, 400.0f), glm::linearRand(-400.0f, 400.0f), glm::linearRand(-400.0f, 400.0f) });

			}

		}

		if (ImGui::Button("Reset Scene")) {

			m_Scene->OnStop();

			m_Scene = m_Project->LoadScene("Physics Scene.lscene");
			m_Project->SetActiveScene(m_Scene);

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
};