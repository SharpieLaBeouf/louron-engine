#include "../Headers/Engine.h"

Engine::Engine()
{
	std::cout << "<---------------------------------------------->" << std::endl;
	std::cout << "<                                              >" << std::endl;
	std::cout << "<              Louron Engine 2020              >" << std::endl;
	std::cout << "<                     [L20]                    >" << std::endl;
	std::cout << "<                  Aaron Sharp                 >" << std::endl;
	std::cout << "<                                              >" << std::endl;
	std::cout << "<---------------------------------------------->" << std::endl << std::endl;

	// 1. Init GLFW
	std::cout << "[L20] GLFW Initialised " << ((glfwInit() == GLFW_TRUE) ? "Successfully!" : "Unsuccessfully!") << std::endl;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	m_Window = new Window("Louron 2020", 800, 800, WindowScreenMode::L20_WINDOW_WINDOWED);
	m_Window->init();
	m_Input = m_Window->getInput();
	std::cout << std::endl;

	// 2. Init GLEW
	GLenum err = glewInit();
	glViewport(0, 0, (GLsizei)m_Window->getWidth(), (GLsizei)m_Window->getHeight());
	
	std::cout << "[L20] GLEW Initialised " << ((err == GLEW_OK) ? "Successfully!" : (const char*)glewGetErrorString(err)) << std::endl;
	std::cout << "[L20] OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	// 3. Init ImGui
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	bool imGuiGLFWErr = ImGui_ImplGlfw_InitForOpenGL(m_Window->getWindow(), true);
	bool imGuiGLEWErr = ImGui_ImplOpenGL3_Init("#version 150");
	std::cout << "[L20] ImGui Initialised " << ((imGuiGLFWErr && imGuiGLEWErr) ? "Successfully!" : "Unsuccessfully!") << std::endl << std::endl;

	// 4. Load Shader and Texture Library
	m_ShaderLib = new ShaderLibrary();
	m_ShaderLib->loadShader("Resources/Shaders/Basic/basic.glsl");
	m_ShaderLib->loadShader("Resources/Shaders/Basic/basic_phong.glsl");
	m_ShaderLib->loadShader("Resources/Shaders/Basic/basic_texture.glsl");
	m_ShaderLib->loadShader("Resources/Shaders/Materials/material_shader_flat.glsl");
	m_ShaderLib->loadShader("Resources/Shaders/Materials/material_shader_phong.glsl");

	m_TextureLib = new TextureLibrary();
	m_TextureLib->loadTexture("Resources/Images/cube_texture.png");
	m_TextureLib->loadTexture("Resources/Images/cube_texture_specular.png");
	m_TextureLib->loadTexture("Resources/Images/grass_texture.jpg");

	// 5. Init Global Scene Manager
	m_InstanceManager = new InstanceManager(m_Window, m_Input, m_ShaderLib, m_TextureLib, &m_States);

}

Engine::~Engine()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_Window->getWindow());
	glfwTerminate();
}
int Engine::run()
{
	ImGuiIO& io = ImGui::GetIO();
	m_States.push_back(std::make_unique<MainMenu>(m_InstanceManager));

	bool demoGUI = false;
	while (!glfwWindowShouldClose(m_Window->getWindow()))
	{
		glfwPollEvents();
				
		if (m_Input->GetKeyUp(GLFW_KEY_ESCAPE)) m_States.pop_back();
		if (m_Input->GetKeyUp(GLFW_KEY_F11)) m_Window->toggleFullscreen();

		// Create New GUI Frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (m_Input->GetKeyUp(GLFW_KEY_F1)) demoGUI = !demoGUI;
		if (demoGUI) ImGui::ShowDemoWindow();

		// Run Scene At Top Of Game State Vector
		if (!m_States.empty())
		{
			m_States.back()->update();
			m_States.back()->draw();
		} else glfwSetWindowShouldClose(m_Window->getWindow(), GLFW_TRUE);

		// Simple FPS Counter
		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, io.DisplaySize.y - 10.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(100.0f, 50.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.35f); 

		if (ImGui::Begin("Simple FPS Overlay", (bool*)0, window_flags))
		{
			ImGui::Text("FPS Counter");
			ImGui::Separator();
			ImGui::Text("%.0f", io.Framerate);
		}
		ImGui::End();


		// Render GUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(m_Window->getWindow());
	}

	return 0;
}


Window* Engine::getWindow()
{
	return m_Window;
}
