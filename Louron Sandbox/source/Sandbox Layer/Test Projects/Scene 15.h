#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

#include "Scene/Scene Serializer.h"

#include "glm/gtc/random.hpp"
#include "glm/gtx/string_cast.hpp"
#include <bitset>

using namespace Louron;

class Scene15 : public TestScene {

private:

	std::shared_ptr<Project> m_Project;
	std::shared_ptr<Scene> m_Scene;

	Window& m_Window;
	InputManager& m_Input;

public:
	Scene15() :
		m_Window(Engine::Get().GetWindow()),
		m_Input(Engine::Get().GetInput())
	{
		L_APP_INFO("Loading Scene 15");

	}

	~Scene15() {
		L_APP_INFO("Unloading Scene 15");

	}

	void OnAttach() override {

		if (!m_Project || !m_Scene) {

			// Load Project and Get Active Scene Handle
			m_Project = Project::LoadProject("Sandbox Project/Sandbox Project.lproj", "Hierarchy System.lscene");
			m_Scene = Project::GetActiveScene();

		}
		m_Scene->OnStart();
	}

	void OnDetach() override {

		m_Scene->OnStop();
	}

	float jump_timer = 0.0f;

	void Update() override {

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().Camera->Update((float)Time::Get().GetDeltaTime());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<CameraComponent>().Camera->GetGlobalPosition());

		m_Scene->OnUpdate();
	}

	void FixedUpdate() override {

		if (!m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().Camera->IsMovementEnabled()) {

			//ROLL THE SPHERE!
			if (m_Input.GetKey(GLFW_KEY_W)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(1.0f, 0.0f, 0.0f) * 10.0f);
			}
			if (m_Input.GetKey(GLFW_KEY_S)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(-1.0f, 0.0f, 0.0f) * 10.0f);
			}
			if (m_Input.GetKey(GLFW_KEY_A)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, -1.0f) * 10.0f);
			}
			if (m_Input.GetKey(GLFW_KEY_D)) {
				m_Scene->FindEntityByName("Ball1").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, 1.0f) * 10.0f);
			}

			//ROLL THE SPHERE!
			if (m_Input.GetKey(GLFW_KEY_UP)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(1.0f, 0.0f, 0.0f) * 10.0f);
			}
			if (m_Input.GetKey(GLFW_KEY_DOWN)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(-1.0f, 0.0f, 0.0f) * 10.0f);
			}
			if (m_Input.GetKey(GLFW_KEY_LEFT)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, -1.0f) * 10.0f);
			}
			if (m_Input.GetKey(GLFW_KEY_RIGHT)) {
				m_Scene->FindEntityByName("Ball2").GetComponent<Rigidbody>().ApplyForce(glm::vec3(0.0f, 0.0f, 1.0f) * 10.0f);
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

		if (ImGui::TreeNode("Ball 1")) {

			Entity ball = m_Scene->FindEntityByName("Ball1");

			DisplayTransformGUI(ball);

			ImGui::Text("Parent Name: %s", ball.GetComponent<HierarchyComponent>().HasParent() ? m_Scene->FindEntityByUUID(ball.GetComponent<HierarchyComponent>().GetParentID()).GetName().c_str() : "None");

			if (ImGui::Button("Toggle Ball 2 As Parent")) {
				if (ball.GetComponent<HierarchyComponent>().HasParent())
					ball.GetComponent<HierarchyComponent>().DetachParent();
				else
					ball.GetComponent<HierarchyComponent>().AttachParent(m_Scene->FindEntityByName("Ball2").GetUUID());
			}

			if (ImGui::Button("Toggle Rigidbody")) {
				if (ball.HasComponent<Rigidbody>())
					ball.RemoveComponent<Rigidbody>();
				else
					ball.AddComponent<Rigidbody>();
			}
			ImGui::SameLine();
			ImGui::Text("%s", ball.HasComponent<Rigidbody>() ? "On" : "Off");

			if (ImGui::Button("Toggle Collider ")) {
				if (ball.HasComponent<SphereCollider>())
					ball.RemoveComponent<SphereCollider>();
				else
					ball.AddComponent<SphereCollider>();
			}
			ImGui::SameLine();
			ImGui::Text("%s", ball.HasComponent<SphereCollider>() ? "On" : "Off");

			if(ball.HasComponent<Rigidbody>())
			{
				PxVec3 velocity = ball.GetComponent<Rigidbody>().GetActor()->GetLinearVelocity();
				ImGui::Text("Velocity: %.2f, %.2f, %.2f", velocity.x, velocity.y, velocity.z);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Ball 2")) {

			Entity ball = m_Scene->FindEntityByName("Ball2");

			DisplayTransformGUI(ball);

			ImGui::Text("Parent Name: %s", ball.GetComponent<HierarchyComponent>().HasParent() ? m_Scene->FindEntityByUUID(ball.GetComponent<HierarchyComponent>().GetParentID()).GetName().c_str() : "None");

			if (ImGui::Button("Toggle Ball 1 As Parent")) {

				if(ball.GetComponent<HierarchyComponent>().HasParent())
					ball.GetComponent<HierarchyComponent>().DetachParent();
				else
					ball.GetComponent<HierarchyComponent>().AttachParent(m_Scene->FindEntityByName("Ball1").GetUUID());

			}

			if (ImGui::Button("Toggle Rigidbody")) {
				if (ball.HasComponent<Rigidbody>())
					ball.RemoveComponent<Rigidbody>();
				else
					ball.AddComponent<Rigidbody>();
			}
			ImGui::SameLine();
			ImGui::Text("%s", ball.HasComponent<Rigidbody>() ? "On" : "Off");

			if (ImGui::Button("Toggle Collider ")) {
				if (ball.HasComponent<SphereCollider>())
					ball.RemoveComponent<SphereCollider>();
				else
					ball.AddComponent<SphereCollider>();
			}
			ImGui::SameLine();
			ImGui::Text("%s", ball.HasComponent<SphereCollider>() ? "On" : "Off");

			if (ball.HasComponent<Rigidbody>())
			{
				PxVec3 velocity = ball.GetComponent<Rigidbody>().GetActor()->GetLinearVelocity();
				ImGui::Text("Velocity: %.2f, %.2f, %.2f", velocity.x, velocity.y, velocity.z);
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("Toggle Camera Movement")) m_Scene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().Camera->ToggleMovement();

		if (ImGui::Button("Reset Scene")) {

			m_Scene->OnStop();

			m_Scene = m_Project->LoadScene("Hierarchy System.lscene");
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

		//temp = trans.GetGlobalPosition();
		//ImGui::Text("Global Position: %.2f, %.2f, %.2f", temp.x, temp.y, temp.z);

		//temp = trans.GetGlobalRotation();
		//ImGui::Text("Global Rotation: %.2f, %.2f, %.2f", temp.x, temp.y, temp.z);

		//temp = trans.GetGlobalScale();
		//ImGui::Text("Global Scale: %.2f, %.2f, %.2f", temp.x, temp.y, temp.z);
	}
};