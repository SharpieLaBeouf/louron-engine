#pragma once

#include "Louron.h"
#include "Test Scene Base.h"
#include "glm/gtc/random.hpp"

class Scene11 : public Scene {

private:

	std::unique_ptr<Louron::Scene> m_Scene;

	Louron::Window& m_Window;
	Louron::InputManager& m_Input;
	Louron::ShaderLibrary& m_ShaderLib;
	Louron::TextureLibrary& m_TextureLib;

	float currentTime = 0;
	float deltaTime = 0;
	float lastTime = 0;
	const int numLights = 20;
	std::vector<float> lightBobOffset;

public:


	Scene11() :
		m_Window(Louron::Engine::Get().GetWindow()),
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		m_Scene(std::make_unique<Louron::Scene>())
	{
		std::cout << "[L20] Loading Scene 11..." << std::endl;

		const auto& resources = m_Scene->GetResources();

		resources->LinkShader(Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));

		resources->LoadMesh("assets/Models/Monkey/Monkey.fbx", resources->Shaders["FP_Material_BP_Shader"]);
		resources->LoadMesh("assets/Models/Monkey/Pink_Monkey.fbx", resources->Shaders["FP_Material_BP_Shader"]);
		resources->LoadMesh("assets/Models/BackPack/BackPack.fbx", resources->Shaders["FP_Material_BP_Shader"]);

		resources->LoadMesh("assets/Models/Sponza/sponza.obj", resources->Shaders["FP_Material_BP_Shader"]);
		resources->LoadMesh("assets/Models/Torch/model/obj/Torch.obj", resources->Shaders["FP_Material_BP_Shader"]);

		Louron::Entity entity = m_Scene->CreateEntity("Sponza");
		entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("sponza"));
		entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("sponza"));
		entity.GetComponent<Louron::Transform>().SetScale(glm::vec3(0.04f));

		entity = m_Scene->CreateEntity("Monkey");
		entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Monkey"));
		entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Monkey"));
		entity.GetComponent<Louron::Transform>().SetPosition({ -15.0f, 4.0f, -6.2f });
		entity.GetComponent<Louron::Transform>().SetRotation({ 0.0f, -45.0f, 0.0f });

		entity = m_Scene->CreateEntity("Pink_Monkey");
		entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Pink_Monkey"));
		entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Pink_Monkey"));
		entity.GetComponent<Louron::Transform>().SetPosition({ -15.0f, 4.0f, 4.2f });
		entity.GetComponent<Louron::Transform>().SetRotation({ 0.0f, -135.0f, 0.0f });

		entity = m_Scene->CreateEntity("BackPack");
		entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("BackPack"));
		entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("BackPack"));
		entity.GetComponent<Louron::Transform>().SetPosition({ -40.0f, 4.0f, -1.2f });
		entity.GetComponent<Louron::Transform>().SetRotation({ 0.0f, 90.0f, 0.0f });

		Louron::PointLightComponent PL_Component;
		for (int i = 0; i < numLights; i++) {
			
			Louron::Entity entity = m_Scene->CreateEntity("Entity " + std::to_string(i));

			PL_Component = entity.AddComponent<Louron::PointLightComponent>();
			PL_Component.diffuse = glm::vec4(1.0f);

			entity.GetComponent<Louron::Transform>().SetPosition({glm::linearRand(-30.0f, 30.0f), glm::linearRand(15.0f, 50.0f), glm::linearRand(-4.2f, 2.2f)});
			entity.GetComponent<Louron::Transform>().SetRotation({glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f)});
			entity.GetComponent<Louron::Transform>().SetScale(glm::vec3(glm::linearRand(0.5f, 2.0f)));
			
			entity.AddComponent<Louron::MeshFilter>().LinkMeshFilter(resources->GetMeshFilter("Torch"));
			entity.AddComponent<Louron::MeshRenderer>().LinkMeshRenderer(resources->GetMeshRenderer("Torch"));

			lightBobOffset.push_back(glm::linearRand(-100.0f, 100.0f));
		}

		// Create Entity for Camera and Set to Primary Camera
		auto& camera = m_Scene->CreateEntity("Main Camera").AddComponent<Louron::CameraComponent>();
		camera.Camera = new Louron::Camera(glm::vec3(0.0f, 0.0f, 10.0f));
		camera.Primary = true;
		camera.Camera->setPitch(-20.0f);
		camera.Camera->setYaw(0.0f);
		camera.Camera->setPosition({ -30.0f, 10.0f, -1.2f });

		auto& pointLight = m_Scene->FindEntityByName("Main Camera").AddComponent<Louron::PointLightComponent>();

		pointLight.position = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().GetPosition(), 1.0f);

		pointLight.ambient =  { 0.2f, 0.2f, 0.2f, 1.0f };
		pointLight.diffuse =  { 0.8f, 0.8f, 0.8f, 1.0f };
		pointLight.specular = { 0.8f, 0.8f, 0.8f, 1.0f };

		pointLight.constant = 1.0f;
		pointLight.linear = 0.09f;
		pointLight.quadratic = 0.032f;

		Louron::Entity dirLight = m_Scene->CreateEntity("Directional Light");
		dirLight.GetComponent<Louron::Transform>().SetRotation({ 50.0f, -30.0f, 0.0f });

		dirLight.AddComponent<Louron::DirectionalLightComponent>();
		dirLight.GetComponent<Louron::DirectionalLightComponent>().ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
		dirLight.GetComponent<Louron::DirectionalLightComponent>().diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
		dirLight.GetComponent<Louron::DirectionalLightComponent>().specular = { 1.0f, 1.0f, 1.0f, 1.0f };

		m_Scene->CreateEntity("Flash Light").AddComponent<Louron::SpotLightComponent>();

	}

	~Scene11() {
		std::cout << "[L20] Unloading Scene 11..." << std::endl;
	}

	void OnAttach() override {
		glEnable(GL_DEPTH_TEST);
		lastTime = (float)glfwGetTime(); 

		m_Scene->OnStart();
	}

	void OnDetach() override {

		glDisable(GL_DEPTH_TEST);
		m_Scene->OnStop();
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->Update(deltaTime);
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetPosition());

		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::Transform>().SetPosition(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::Transform>().GetPosition());
		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

		if (m_Input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
			for (int i = 0; i < numLights; i++) {
				Louron::Entity entity = m_Scene->FindEntityByName("Entity " + std::to_string(i));
				entity.GetComponent<Louron::Transform>().SetPosition({ glm::linearRand(-30.0f, 30.0f), glm::linearRand(15.0f, 50.0f), glm::linearRand(-4.2f, 2.2f)});
				entity.GetComponent<Louron::Transform>().SetRotation({ glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f) });
				entity.GetComponent<Louron::Transform>().SetScale(glm::vec3(glm::linearRand(0.5f, 2.0f)));
			}
		}

		for (int i = 0; i < numLights; i++) {
			
			// Each Light has their own bobbing height
			float bobbingOffset = sin(currentTime + lightBobOffset[i]) * deltaTime;

			Louron::Entity entity = m_Scene->FindEntityByName("Entity " + std::to_string(i));
			entity.GetComponent<Louron::Transform>().Translate({ 0.0f, bobbingOffset, 0.0f});
		}

		Draw();
	}

	glm::ivec2 tileID{ 0, 0 };

	void UpdateGUI() override {

		m_Scene->OnUpdateGUI();

		ImGui::Begin("Application Scene Control", (bool*)0,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse
		);

		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(200.0f, 100.0f));

		ImGui::Separator();

		ImGui::Text("Light Count: %i", numLights);

		if (ImGui::TreeNode("Colours"))
		{
			ImGui::ColorPicker4("Background", glm::value_ptr(back_colour));
			ImGui::TreePop();
		}

		ImGui::End();

		////Frustum Plane Algorithm Testing
		//glm::vec4 frustumPlanes[6]{};
		//bool frustumPlanesChecks[6]{ false };
		//glm::vec2 tileNumber{ (float)m_Window.GetWidth() / 16.0f, glm::ceil((float)m_Window.GetHeight() / 16.0f) };

		//glm::vec2 negativeStep = (2.0f * glm::vec2(tileID)) / tileNumber;
		//glm::vec2 positiveStep = (2.0f * glm::vec2(tileID + glm::ivec2(1, 1))) / tileNumber;

		//frustumPlanes[0] = glm::vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		//frustumPlanes[1] = glm::vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		//frustumPlanes[2] = glm::vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		//frustumPlanes[3] = glm::vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		//frustumPlanes[4] = glm::vec4(0.0, 0.0, -1.0, 0.1); // Near
		//frustumPlanes[5] = glm::vec4(0.0, 0.0, 1.0, 100.0); // Far

		//glm::mat4 view = m_Scene->GetPrimaryCameraEntity().GetComponent<Louron::CameraComponent>().Camera->GetViewMatrix();
		//glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)m_Window.GetWidth() / (float)m_Window.GetHeight(), 0.1f, 100.0f);

		//glm::mat4 viewProj = proj * view;
		//for (int i = 0; i < 4; i++) {
		//	frustumPlanes[i] = frustumPlanes[i] * viewProj;
		//	frustumPlanes[i] /= glm::length(glm::vec3(frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z));
		//}

		//frustumPlanes[4] = frustumPlanes[4] * view;
		//frustumPlanes[4] /= glm::length(glm::vec3(frustumPlanes[4].x, frustumPlanes[4].y, frustumPlanes[4].z));
		//frustumPlanes[5] = frustumPlanes[5] * view;
		//frustumPlanes[5] /= glm::length(glm::vec3(frustumPlanes[5].x, frustumPlanes[5].y, frustumPlanes[5].z));

		//glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		//// We check if the light exists in our frustum
		//float distance = 0.0f;
		//for (int j = 0; j < 6; j++) {
		//	distance = glm::dot(position, frustumPlanes[j]);

		//	// If one of the tests fails, then there is no intersection
		//	if (distance <= 0.0) {
		//		break;
		//	}
		//	frustumPlanesChecks[j] = true;
		//}



		//if (ImGui::Begin("Frustum Plane Algorithm Testing", (bool*)0))
		//{

		//	ImGuiIO& io = ImGui::GetIO();
		//	glm::vec2 menuSize = { 600.0f, 400.0f };
		//	ImGui::SetWindowSize(ImVec2(menuSize.x, menuSize.y), ImGuiCond_Always);
		//	ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 2 - menuSize.x / 2, io.DisplaySize.y / 2 - menuSize.y / 2), ImGuiCond_Always);

		//	ImGui::Text("Frustum Plane Algorithm Testing");
		//	ImGui::Separator();

		//	int* tempX = &tileID.x;
		//	int* tempY = &tileID.y;

		//	ImGui::DragInt("Tile ID X", tempX, 0.5f, 0, 99);
		//	ImGui::DragInt("Tile ID Y", tempY, 0.5f, 0, 56);

		//	ImGui::Text("Left Frustum Plane: %.2f, %.2f, %.2f, %.2f",   frustumPlanes[0].x, frustumPlanes[0].y, frustumPlanes[0].z, frustumPlanes[0].w);
		//	frustumPlanesChecks[0] ? ImGui::Text("True") : ImGui::Text("False");
		//	ImGui::Text("Right Frustum Plane: %.2f, %.2f, %.2f, %.2f",  frustumPlanes[1].x, frustumPlanes[1].y, frustumPlanes[1].z, frustumPlanes[1].w);
		//	frustumPlanesChecks[1] ? ImGui::Text("True") : ImGui::Text("False");
		//	ImGui::Text("Bottom Frustum Plane: %.2f, %.2f, %.2f, %.2f", frustumPlanes[2].x, frustumPlanes[2].y, frustumPlanes[2].z, frustumPlanes[2].w);
		//	frustumPlanesChecks[2] ? ImGui::Text("True") : ImGui::Text("False");
		//	ImGui::Text("Top Frustum Plane: %.2f, %.2f, %.2f, %.2f",    frustumPlanes[3].x, frustumPlanes[3].y, frustumPlanes[3].z, frustumPlanes[3].w);
		//	frustumPlanesChecks[3] ? ImGui::Text("True") : ImGui::Text("False");
		//	ImGui::Text("Near Frustum Plane: %.2f, %.2f, %.2f, %.2f",    frustumPlanes[4].x, frustumPlanes[4].y, frustumPlanes[4].z, frustumPlanes[4].w);
		//	frustumPlanesChecks[4] ? ImGui::Text("True") : ImGui::Text("False");
		//	ImGui::Text("Far Frustum Plane: %.2f, %.2f, %.2f, %.2f",    frustumPlanes[5].x, frustumPlanes[5].y, frustumPlanes[5].z, frustumPlanes[5].w);
		//	frustumPlanesChecks[5] ? ImGui::Text("True") : ImGui::Text("False");

		//	// If greater than zero, then it is a visible light
		//	if (distance > 0.0) {
		//		ImGui::Text("Within Frustum!");
		//	}
		//}
		//ImGui::End();


	}

private:

	glm::vec4 back_colour = { 0.5294f, 0.8078f, 0.9215f, 1.0f };

	void Draw() override {

		glClearColor(back_colour.r, back_colour.g, back_colour.b, back_colour.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Scene->OnUpdate();

	}
};