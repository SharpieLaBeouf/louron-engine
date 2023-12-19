// CUSTOM HEADERS
#include "Engine Manager.h"
#include "../Tests/MainMenu.h"

#include <iostream>
#include <conio.h>
#include <stack>
#include <memory>

LouronEngine* game;
std::vector<std::unique_ptr<State>>* states;

void GUI_FPS(ImGuiIO& io);

int main()
{
	if (!game->StartEngine())
		return -1;

	states->push_back(std::make_unique<MainMenu>(game, states));

	ImGuiIO& io = ImGui::GetIO();
	while (!glfwWindowShouldClose(game->getWindowInstance()->getWindow())) {
		game->EngineLoop();

		std::cout << "WERE IN GAME APPLICATION LOOP TO PROCESS ALL APPLICATION REQUIREMENTS" << std::endl;

		glfwPollEvents();

		if (game->getInputInstance()->GetKeyUp(GLFW_KEY_ESCAPE)) states->pop_back();
		if (game->getInputInstance()->GetKeyUp(GLFW_KEY_F11)) game->getWindowInstance()->toggleFullscreen();

		// Create New GUI Frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Run Scene At Top Of Game State Vector
		if (!states->empty())
		{
			states->back()->update();
			states->back()->draw();
		}
		else glfwSetWindowShouldClose(game->getWindowInstance()->getWindow(), GLFW_TRUE);

		GUI_FPS(io);

	}

	return 0;
}

void GUI_FPS(ImGuiIO& io) {
	glfwPollEvents();

	if (game->getInputInstance()->GetKeyUp(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(game->getWindowInstance()->getWindow(), GLFW_TRUE);
	if (game->getInputInstance()->GetKeyUp(GLFW_KEY_F11)) game->getWindowInstance()->toggleFullscreen();

	// Create New GUI Frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

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
	glfwSwapBuffers(game->getWindowInstance()->getWindow());
}