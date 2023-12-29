#pragma once

#include "Louron.h"
#include "Test Scene Base.h"

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

public:


	Scene11() :
		m_Window(Louron::Engine::Get().GetWindow()),
		m_Input(Louron::Engine::Get().GetInput()),
		m_ShaderLib(Louron::Engine::Get().GetShaderLibrary()),
		m_TextureLib(Louron::Engine::Get().GetTextureLibrary()),
		m_Scene(std::make_unique<Louron::Scene>())
	{
		std::cout << "[L20] Opening Scene 11..." << std::endl;

		m_ShaderLib.LoadShader("assets/Shaders/Forward+/FP_Material_Shader.glsl");
		m_ShaderLib.LoadShader("assets/Shaders/Forward+/FP_Light_Culling.comp", true);

		// Create Entity for Monkey and Load Applicable Model
		m_Scene->CreateEntity("Monkey").AddComponent<Louron::MeshRendererComponent>().loadModel("assets/Models/Monkey/Pink_Monkey.fbx", "FP_Material_Shader");

		// Create Entity for Camera and Set to Primary Camera
		auto& camera = m_Scene->CreateEntity("Main Camera").AddComponent<Louron::CameraComponent>();
		camera.Camera = new Louron::Camera(glm::vec3(0.0f, 0.0f, 10.0f));
		camera.Primary = true;

		auto& pointLight = m_Scene->CreateEntity("Point Light 1").AddComponent<Louron::PointLightComponent>();

		pointLight.position = m_Scene->FindEntityByName("Point Light 1").GetComponent<Louron::TransformComponent>().position;

		pointLight.ambient = { 0.05f, 0.05f, 0.05f };
		pointLight.diffuse = { 0.05f, 0.05f, 0.05f };
		pointLight.specular = { 0.05f, 0.05f, 0.05f };

		pointLight.Constant = 1.0f;
		pointLight.Linear = 0.09f;
		pointLight.Quadratic = 0.032f;

	}

	~Scene11() {
		std::cout << "[L20] Closing Scene 11..." << std::endl;


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

		Draw();
	}

	void UpdateGUI() override {

	}

private:

	void Draw() override {

		glClearColor(211.0f / 255.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Scene->OnUpdate();

	}
};