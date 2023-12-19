#include "Engine Manager.h"

LouronEngine::LouronEngine() { }

LouronEngine::~LouronEngine() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_WindowManager->getWindow());
	glfwTerminate();

	delete m_WindowManager;
	delete m_InputManager;
	delete m_ShaderLibrary;
	delete m_TextureLibrary;
}

int LouronEngine::StartEngine()
{
	// 1. Init GLFW
	if (glfwInit() == GLFW_FALSE) {
		std::cout << "[L20] GLFW Initialised Unsuccessfully!\n[L20] Exiting Now..." << std::endl;
		return -1;
	}
	std::cout << "[L20] GLFW Initialised Successfully!" << std::endl;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_WindowManager = new Window("Louron 2020", 800, 800, WindowScreenMode::L20_WINDOW_WINDOWED);
	m_WindowManager->init();
	m_InputManager = m_WindowManager->getInput();
	std::cout << std::endl;

	// 2. Init GLAD
	if (gladLoadGL() == GL_FALSE) {
		std::cout << "[L20] GLAD Initialised Unsuccessfully!\n[L20] Exiting Now..." << std::endl;
		return -1;
	}
	std::cout << "[L20] GLAD Initialised Successfully!" << std::endl;
	std::cout << "[L20] OpenGL Version: " << (const char*)glGetString(GL_VERSION) << std::endl << std::endl;
	glViewport(0, 0, (GLsizei)m_WindowManager->getWidth(), (GLsizei)m_WindowManager->getHeight());

	// 3. Init ImGui
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	bool imGuiGLFWErr = ImGui_ImplGlfw_InitForOpenGL(m_WindowManager->getWindow(), true);
	bool imGuiGLEWErr = ImGui_ImplOpenGL3_Init("#version 450");
	std::cout << "[L20] ImGui Initialised " << ((imGuiGLFWErr && imGuiGLEWErr) ? "Successfully!" : "Unsuccessfully!\n[L20] Exiting Now...") << std::endl << std::endl;
	if (!imGuiGLFWErr || !imGuiGLEWErr) return -1;

	// 4. Load Shader Library
	m_ShaderLibrary = new ShaderLibrary();
	m_ShaderLibrary->loadShader("assets/Shaders/Basic/basic.glsl");
	m_ShaderLibrary->loadShader("assets/Shaders/Basic/basic_phong.glsl");
	m_ShaderLibrary->loadShader("assets/Shaders/Basic/basic_texture.glsl");
	m_ShaderLibrary->loadShader("assets/Shaders/Materials/material_shader_flat.glsl");
	m_ShaderLibrary->loadShader("assets/Shaders/Materials/material_shader_phong.glsl");
	std::cout << std::endl;

	// 5. Load Texture Library
	m_TextureLibrary = new TextureLibrary();
	m_TextureLibrary->loadTexture("assets/Images/cube_texture.png");
	m_TextureLibrary->loadTexture("assets/Images/cube_texture_specular.png");
	m_TextureLibrary->loadTexture("assets/Images/grass_texture.jpg");

	return 1;
}

int LouronEngine::EngineLoop()
{
	std::cout << "WERE IN GAME ENGINE LOOP TO PROCESS ALL ENGINE REQUIREMENTS" << std::endl;

	return 1;
}
