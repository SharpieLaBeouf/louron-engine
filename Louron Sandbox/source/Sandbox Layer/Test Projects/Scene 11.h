#pragma once

#include "Louron.h"
#include "Test Scene Base.h"
#include "glm/gtc/random.hpp"

class Scene11 : public TestScene {

private:

	std::shared_ptr<Louron::Scene> m_Scene;

	Louron::Window& m_Window;
	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
	const int numLights = 25;
	std::vector<float> lightBobOffset;

	std::shared_ptr<Louron::ForwardPlusPipeline> m_Pipeline;

	struct {

		bool lightActive = true;
		float lightRadius = 10.0f;
		float lightIntensity = 1.0f;
		glm::vec3 lightPosition = glm::vec3(0.0f, 2.0f, 0.0f);

	} m_PointLightProps;

public:


	Scene11() :
		m_Window(Louron::Engine::Get().GetWindow()),
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		m_Pipeline(nullptr),
		m_Scene(nullptr)
	{
		L_APP_INFO("Loading Scene 11");

	}

	~Scene11() {
		L_APP_INFO("Unloading Scene 11");
	}

	void OnAttach() override {
		lastTime = (float)glfwGetTime(); 

		if (!m_Scene || !m_Pipeline) {

			// Scene Configuration Setup
			m_Pipeline = std::make_shared<Louron::ForwardPlusPipeline>();
			m_Scene = std::make_shared<Louron::Scene>(Louron::L_RENDER_PIPELINE::FORWARD_PLUS);

			// Scene ResourcesSetup
			const auto& resources = m_Scene->GetResources();
			resources->LinkShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));
			resources->LoadMesh("assets/Models/Monkey/Monkey.fbx", resources->Shaders["FP_Material_BP_Shader"]);
			resources->LoadMesh("assets/Models/Monkey/Pink_Monkey.fbx", resources->Shaders["FP_Material_BP_Shader"]);
			resources->LoadMesh("assets/Models/BackPack/BackPack.fbx", resources->Shaders["FP_Material_BP_Shader"]);
			resources->LoadMesh("assets/Models/Sponza/sponza.obj", resources->Shaders["FP_Material_BP_Shader"]);
			resources->LoadMesh("assets/Models/Torch/model/obj/Torch.obj", resources->Shaders["FP_Material_BP_Shader"]);

			// Sponza
			Louron::Entity entity = m_Scene->CreateEntity("Sponza");
			entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("sponza"));
			entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("sponza"));
			entity.GetComponent<Louron::Transform>().SetScale(glm::vec3(0.04f));

			// Monkey
			entity = m_Scene->CreateEntity("Monkey");
			entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Monkey"));
			entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Monkey"));
			entity.GetComponent<Louron::Transform>().SetPosition({ -15.0f, 4.0f, -6.2f });
			entity.GetComponent<Louron::Transform>().SetRotation({ 0.0f, -45.0f, 0.0f });

			// Pink Monkey
			entity = m_Scene->CreateEntity("Pink_Monkey");
			entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Pink_Monkey"));
			entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Pink_Monkey"));
			entity.GetComponent<Louron::Transform>().SetPosition({ -15.0f, 4.0f, 4.2f });
			entity.GetComponent<Louron::Transform>().SetRotation({ 0.0f, -135.0f, 0.0f });

			// BackPack
			entity = m_Scene->CreateEntity("BackPack");
			entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("BackPack"));
			entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("BackPack"));
			entity.GetComponent<Louron::Transform>().SetPosition({ -40.0f, 4.0f, -1.2f });
			entity.GetComponent<Louron::Transform>().SetRotation({ 0.0f, 90.0f, 0.0f });

			// Point Lights
			Louron::PointLightComponent PL_Component;
			for (int i = 0; i < numLights; i++) {

				Louron::Entity entity = m_Scene->CreateEntity("Light Source " + std::to_string(i));

				entity.AddComponent<Louron::PointLightComponent>();
				entity.GetComponent<Louron::PointLightComponent>().lightProperties.radius = 20.0f;
				entity.GetComponent<Louron::Transform>().SetPosition({ glm::linearRand(-30.0f, 30.0f), glm::linearRand(15.0f, 50.0f), glm::linearRand(-4.2f, 2.2f) });
				entity.GetComponent<Louron::Transform>().SetRotation({ glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f) });
				entity.GetComponent<Louron::Transform>().SetScale(glm::vec3(glm::linearRand(0.5f, 2.0f)));

				entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Torch"));
				entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Torch"));

				lightBobOffset.push_back(glm::linearRand(-100.0f, 100.0f));
			}

			// Main Camera
			auto& camera = m_Scene->CreateEntity("Main Camera").AddComponent<Louron::CameraComponent>();
			camera.Camera = std::make_shared<Louron::Camera>(glm::vec3(-30.0f, 10.0f, -1.2f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, -20.0f);
			camera.Camera->MouseToggledOff = false;

			camera.Primary = true;
			camera.ClearFlags = Louron::CameraClearFlags::SKYBOX;

			m_Scene->FindEntityByName("Main Camera").AddComponent<Louron::SpotLightComponent>();
			Louron::SkyboxComponent& skybox = m_Scene->FindEntityByName("Main Camera").AddComponent<Louron::SkyboxComponent>();

			std::array<std::filesystem::path, 6> skyboxFaces
			{
				std::filesystem::path("Sandbox Project/Assets/Kloppenheim Skybox/right.png"),
				std::filesystem::path("Sandbox Project/Assets/Kloppenheim Skybox/left.png"),
				std::filesystem::path("Sandbox Project/Assets/Kloppenheim Skybox/top.png"),
				std::filesystem::path("Sandbox Project/Assets/Kloppenheim Skybox/bottom.png"),
				std::filesystem::path("Sandbox Project/Assets/Kloppenheim Skybox/back.png"),
				std::filesystem::path("Sandbox Project/Assets/Kloppenheim Skybox/front.png")
			};
			skybox.Material->LoadSkybox(skyboxFaces);

			// Directional Light
			Louron::Entity dirLight = m_Scene->CreateEntity("Directional Light");
			dirLight.AddComponent<Louron::DirectionalLightComponent>();
			dirLight.GetComponent<Louron::DirectionalLightComponent>().ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
			dirLight.GetComponent<Louron::DirectionalLightComponent>().diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
			dirLight.GetComponent<Louron::DirectionalLightComponent>().specular = { 1.0f, 1.0f, 1.0f, 1.0f };
			dirLight.GetComponent<Louron::Transform>().SetRotation({ 50.0f, -30.0f, 0.0f });
		}

		m_Scene->OnStart();
	}

	void OnDetach() override {

		m_Scene->OnStop();
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->Update(deltaTime);
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetGlobalPosition());
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

		if (m_Input.GetKeyDown(GLFW_KEY_F)) {
			Louron::SpotLightComponent& spotLight = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::SpotLightComponent>();
			spotLight.lightProperties.active = (spotLight.lightProperties.active == GL_TRUE) ? GL_FALSE : GL_TRUE;
		}

		// Update Cherry Picked Light Properties
		m_Scene->FindEntityByName("Light Source 0").GetComponent<Louron::PointLightComponent>().lightProperties.radius = m_PointLightProps.lightRadius;
		m_Scene->FindEntityByName("Light Source 0").GetComponent<Louron::PointLightComponent>().lightProperties.intensity = m_PointLightProps.lightIntensity;
		m_Scene->FindEntityByName("Light Source 0").GetComponent<Louron::PointLightComponent>().lightProperties.active = m_PointLightProps.lightActive;
		m_Scene->FindEntityByName("Light Source 0").GetComponent<Louron::Transform>().SetPosition(m_PointLightProps.lightPosition);
		
		// Randomly Update Transforms of Point Light Sources
		if (m_Input.GetKeyDown(GLFW_KEY_ENTER)) {
			for (int i = 0; i < numLights; i++) {
				Louron::Entity entity = m_Scene->FindEntityByName("Light Source " + std::to_string(i));
				entity.GetComponent<Louron::Transform>().SetPosition({ glm::linearRand(-30.0f, 30.0f), glm::linearRand(15.0f, 50.0f), glm::linearRand(-4.2f, 2.2f) });
				entity.GetComponent<Louron::Transform>().SetRotation({ glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f) });
				entity.GetComponent<Louron::Transform>().SetScale(glm::vec3(glm::linearRand(0.5f, 2.0f)));
			}
		}

		// Bob Lights Up and Down
		for (int i = 1; i < numLights; i++) {

			// Each Light has their own bobbing height
			float bobbingOffset = sin(currentTime + lightBobOffset[i]) * deltaTime;
			m_Scene->FindEntityByName("Light Source " + std::to_string(i)).GetComponent<Louron::Transform>().TranslateY(bobbingOffset);
		}

		Draw();
	}

	glm::ivec2 tileID{ 0, 0 };

	void UpdateGUI() override {

		m_Scene->OnUpdateGUI();

		ImGui::Begin("Application Scene Control", (bool*)0,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse
		);

		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

		ImGui::Separator();

		ImGui::Text("Light Count: %i", numLights);

		if (ImGui::TreeNode("Colours"))
		{
			ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Point Light Properties")) {
			ImGui::DragFloat("Radius", &m_PointLightProps.lightRadius, 0.05f);
			ImGui::DragFloat("Intensity", &m_PointLightProps.lightIntensity, 0.05f);
			ImGui::DragFloat3("Position", &m_PointLightProps.lightPosition[0], 0.1f);

			if (ImGui::Button("Toggle Active")) m_PointLightProps.lightActive = !m_PointLightProps.lightActive;

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Profiling")) {

			for (auto& result : Louron::Profiler::Get().GetResults()) {

				char label[128];
				strcpy_s(label, result.second.Name);
				strcat_s(label, " %.3fms");

				ImGui::Text(label, result.second.Time);
			}

			ImGui::TreePop();
		}

		ImGui::End();

	}

private:

	glm::vec4 back_colour = { 0.5294f, 0.8078f, 0.9215f, 1.0f };

	void Draw() override {

		Louron::Renderer::ClearColour(back_colour);
		
		m_Scene->OnUpdate();

	}
};