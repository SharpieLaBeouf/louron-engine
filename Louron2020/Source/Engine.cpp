#include "../Headers/Engine.h"



Engine::Engine()
{
	// 1. Init GLFW
	std::cout << "[L20] GLFW Initialised " << ((glfwInit() == GLFW_TRUE) ? "Successfully!" : "Unsuccessfully!") << std::endl;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	int screenMode = 0;
	/*while (true)
	{
		std::cout << "[L20] GLFW Enter a ScreenMode: ";
		std::cin >> screenMode;
		if (screenMode != 0)
			if (screenMode != 1)
				if (screenMode != 2)
					if (screenMode != 3)
						std::cout << "[L20] GLFW Please Enter a Correct ScreenMode!" << std::endl;
					else break;
				else break;
			else break;
		else break;
	}*/
	m_Window = new Window("Louron 2020", 800, 800, screenMode);
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
	std::cout << "[L20] ImGui Initialised " << ((imGuiGLFWErr && imGuiGLEWErr) ? "Successfully!" : "Unsuccessfully!") << std::endl;

#ifndef _DEBUG
	m_fpsToggled = false;
#endif

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
	m_States.push(std::make_unique<State::MainMenu>(&m_States, m_Window));

	bool demoGUI = false;
	while (!glfwWindowShouldClose(m_Window->getWindow()))
	{
		glfwPollEvents();

		

		int width, height;
		glfwGetWindowSize(m_Window->getWindow(), &width, &height);
		m_Window->setWidth((float)width);
		m_Window->setHeight((float)height);
		
		if (m_Input->GetKeyUp(GLFW_KEY_ESCAPE)) m_States.pop();
		if (m_Input->GetKeyUp(GLFW_KEY_F11)) m_Window->toggleFullscreen();
		if (m_Input->GetKeyDown(GLFW_KEY_F10)) m_Window->toggleConsole();
		if (m_Input->GetKeyUp(GLFW_KEY_F9)) m_fpsToggled = !m_fpsToggled;

		// Create New GUI Frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (m_Input->GetKeyUp(GLFW_KEY_F1)) demoGUI = !demoGUI;
		if (demoGUI) ImGui::ShowDemoWindow();

		// Run Scene At Top Of Game State Stack
		if (!m_States.empty())
		{
			m_States.top()->update();
			m_States.top()->draw();
		} else glfwSetWindowShouldClose(m_Window->getWindow(), GLFW_TRUE);

		if (m_fpsToggled)
		{
			ImGuiWindowFlags window_flags =
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoMove;
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, io.DisplaySize.y - 10.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
			ImGui::SetNextWindowSize(ImVec2(100.0f, 50.0f), ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

			if (ImGui::Begin("Simple FPS Overlay", (bool*)0, window_flags))
			{
				ImGui::Text("FPS Counter");
				ImGui::Separator();
				ImGui::Text("%.0f", io.Framerate);
			}
			ImGui::End();
		}


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
