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
	const int numCubes = 50;

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

		resources->LoadShader("FP_Material_BP_Shader");

		resources->LoadMesh("assets/Models/Cube/Cube.fbx", Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));
		resources->LoadMesh("assets/Models/Monkey/Monkey.fbx", Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));
		resources->LoadMesh("assets/Models/BackPack/BackPack.fbx", Louron::Engine::Get().GetShaderLibrary().GetShader("FP_Material_BP_Shader"));

		for (int i = 0; i < numCubes; i++) {
			
			Louron::Entity entity = m_Scene->CreateEntity("Entity " + std::to_string(i));
			Louron::MaterialComponent& materials = entity.AddComponent<Louron::MaterialComponent>("FP_Material_BP_Shader");

			entity.GetComponent<Louron::TransformComponent>().position = { glm::linearRand(-10.0f, 10.0f), glm::linearRand(-10.0f, 10.0f), glm::linearRand(-20.0f, -1.0f) };
			entity.GetComponent<Louron::TransformComponent>().rotation = { glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f) };
			entity.GetComponent<Louron::TransformComponent>().scale = glm::vec3(glm::linearRand(0.5f, 2.0f));
			
			if (i <= numCubes / 2) {
				entity.AddComponent<Louron::MeshComponent>(resources->GetMesh("Monkey"));
				entity.GetComponent<Louron::MaterialComponent>().LinkMeshMaterials(resources->GetMesh("Monkey"));
			}
			else {
				entity.AddComponent<Louron::MeshComponent>(resources->GetMesh("Cube"));
				entity.GetComponent<Louron::MaterialComponent>().LinkMeshMaterials(resources->GetMesh("Cube"));

				if (glm::linearRand(-1.0f, 1.0f) > 0.0f)
				{
					materials.Materials[0]->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib.GetTexture("cube_texture"));
					materials.Materials[0]->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_SPECULAR_MAP, m_TextureLib.GetTexture("cube_texture_specular"));
				}
				else
				{
					materials.Materials[0]->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_DIFFUSE_MAP, m_TextureLib.GetTexture("stone_texture"));
					materials.Materials[0]->AddTextureMap(Louron::TextureMapType::L20_TEXTURE_SPECULAR_MAP, m_TextureLib.GetTexture("stone_texture_specular"));
				}
			}
		}


		// Create Entity for Camera and Set to Primary Camera
		auto& camera = m_Scene->CreateEntity("Main Camera").AddComponent<Louron::CameraComponent>();
		camera.Camera = new Louron::Camera(glm::vec3(0.0f, 0.0f, 10.0f));
		camera.Primary = true;

		auto& pointLight = m_Scene->FindEntityByName("Main Camera").AddComponent<Louron::PointLightComponent>();

		pointLight.position = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::TransformComponent>().position, 1.0f);

		pointLight.ambient =  { 0.2f, 0.2f, 0.2f, 1.0f };
		pointLight.diffuse =  { 0.8f, 0.8f, 0.8f, 1.0f };
		pointLight.specular = { 0.8f, 0.8f, 0.8f, 1.0f };

		pointLight.constant = 1.0f;
		pointLight.linear = 0.09f;
		pointLight.quadratic = 0.032f;

		Louron::Entity dirLight = m_Scene->CreateEntity("Directional Light");
		dirLight.AddComponent<Louron::DirectionalLightComponent>();

		dirLight.GetComponent<Louron::DirectionalLightComponent>().direction = { -0.2f, -1.0f, -0.3f, 1.0f };
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
	}

	void OnDetach() override {

		glDisable(GL_DEPTH_TEST);
	}

	void Update() override {

		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update Camera Component
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->Update(deltaTime);
		m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::TransformComponent>().position = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetPosition();

		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::TransformComponent>().position = m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::TransformComponent>().position;
		m_Scene->FindEntityByName("Flash Light").GetComponent<Louron::SpotLightComponent>().direction = glm::vec4(m_Scene->FindEntityByName("Main Camera").GetComponent<Louron::CameraComponent>().Camera->GetCameraDirection(), 1.0f);

		if (m_Input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {

			for (int i = 0; i < numCubes; i++) {
				Louron::Entity entity = m_Scene->FindEntityByName("Entity " + std::to_string(i));
				entity.GetComponent<Louron::TransformComponent>().position = { glm::linearRand(-10.0f, 10.0f), glm::linearRand(-10.0f, 10.0f), glm::linearRand(-20.0f, -1.0f) };
				entity.GetComponent<Louron::TransformComponent>().rotation = { glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f), glm::linearRand(-180.0f, 180.0f) };
				entity.GetComponent<Louron::TransformComponent>().scale = glm::vec3(glm::linearRand(0.5f, 2.0f));
			}
		}

		Draw();
	}

	glm::ivec2 tileID{ 0, 0 };

	void UpdateGUI() override {

		//Frustum Plane Algorithm Testing
		glm::vec4 frustumPlanes[6]{};
		bool frustumPlanesChecks[6]{ false };
		glm::vec2 tileNumber{ (float)m_Window.GetWidth() / 16.0f, glm::ceil((float)m_Window.GetHeight() / 16.0f) };

		glm::vec2 negativeStep = (2.0f * glm::vec2(tileID)) / tileNumber;
		glm::vec2 positiveStep = (2.0f * glm::vec2(tileID + glm::ivec2(1, 1))) / tileNumber;

		frustumPlanes[0] = glm::vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = glm::vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = glm::vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = glm::vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = glm::vec4(0.0, 0.0, -1.0, 0.1); // Near
		frustumPlanes[5] = glm::vec4(0.0, 0.0, 1.0, 100.0); // Far

		glm::mat4 view = m_Scene->GetPrimaryCameraEntity().GetComponent<Louron::CameraComponent>().Camera->GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)m_Window.GetWidth() / (float)m_Window.GetHeight(), 0.1f, 100.0f);

		glm::mat4 viewProj = proj * view;
		for (int i = 0; i < 4; i++) {
			frustumPlanes[i] = frustumPlanes[i] * viewProj;
			frustumPlanes[i] /= glm::length(glm::vec3(frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z));
		}

		frustumPlanes[4] = frustumPlanes[4] * view;
		frustumPlanes[4] /= glm::length(glm::vec3(frustumPlanes[4].x, frustumPlanes[4].y, frustumPlanes[4].z));
		frustumPlanes[5] = frustumPlanes[5] * view;
		frustumPlanes[5] /= glm::length(glm::vec3(frustumPlanes[5].x, frustumPlanes[5].y, frustumPlanes[5].z));

		glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// We check if the light exists in our frustum
		float distance = 0.0f;
		for (int j = 0; j < 6; j++) {
			distance = glm::dot(position, frustumPlanes[j]);

			// If one of the tests fails, then there is no intersection
			if (distance <= 0.0) {
				break;
			}
			frustumPlanesChecks[j] = true;
		}


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

	void Draw() override {

		glClearColor(0.0f, 0.0f, 0.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Scene->OnUpdate();

	}
};