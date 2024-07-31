#include "GuiLayer.h"

// Louron Core Headers
#include "Engine.h"
#include "Logging.h"
#include "../Debug/Assert.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Louron {

	GuiLayer::GuiLayer() : Layer("GuiLayer") { }

	void GuiLayer::OnAttach() {

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		style.FrameRounding = 4.0f;
		style.FrameBorderSize = 1.0f;

		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ColorButtonPosition = ImGuiDir_Left;

		SetDarkThemeColors();

		Engine& app = Engine::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		bool imGuiGLFWErr = ImGui_ImplGlfw_InitForOpenGL(window, true);
		bool imGuiGLEWErr = ImGui_ImplOpenGL3_Init("#version 450");

		if (imGuiGLFWErr && imGuiGLEWErr)
			L_CORE_INFO("ImGui Initialised Successfully");
		else {
			L_CORE_FATAL("ImGui Initialised Unsuccessfully");
			Engine::Get().Close();
		}
	}

	void GuiLayer::OnDetach() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GuiLayer::Begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void GuiLayer::End() {
		ImGuiIO& io = ImGui::GetIO();
		Engine& app = Engine::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void GuiLayer::SetDarkThemeColors() {
		auto& colors = ImGui::GetStyle().Colors;

		// Text
		colors[ImGuiCol_Text]					= ImVec4{ 0.875f, 0.875f, 0.875f, 1.0f };
		colors[ImGuiCol_TextSelectedBg]			= ImVec4{ 1.0f, 0.842f, 0.227f, 0.349f };

		// Separator
		colors[ImGuiCol_Separator]				= ImVec4{ 0.0f, 0.0f, 0.0f, 1.0f };

		// Windows
		colors[ImGuiCol_WindowBg]				= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };
		colors[ImGuiCol_DockingPreview]			= ImVec4{ 1.0f, 0.842f, 0.227f, 0.381f };

		// Borders
		colors[ImGuiCol_Border]					= ImVec4{ 0.12f, 0.12f, 0.12f, 1.0f};
		colors[ImGuiCol_BorderShadow]			= ImVec4{ 0.221f, 0.221f, 0.221f, 1.0f};

		// Headers
		colors[ImGuiCol_Header]					= ImVec4{ 1.0f, 0.842f, 0.227f, 0.416f };
		colors[ImGuiCol_HeaderHovered]			= ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive]			= ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button]					= ImVec4{ 0.165f, 0.165f, 0.165f, 1.0f };
		colors[ImGuiCol_ButtonHovered]			= ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colors[ImGuiCol_ButtonActive]			= ImVec4{ 0.588f, 0.588f, 0.588f, 1.0f };
		colors[ImGuiCol_CheckMark]				= ImVec4{ 1.0f, 0.842f, 0.227f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg]				= ImVec4{ 0.165f, 0.165f, 0.165f, 1.0f };
		colors[ImGuiCol_FrameBgHovered]			= ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive]			= ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab]					= ImVec4{ 0.141f, 0.141f, 0.141f, 1.0f };
		colors[ImGuiCol_TabHovered]				= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };

		colors[ImGuiCol_TabSelected]			= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };
		colors[ImGuiCol_TabSelectedOverline]	= ImVec4{ 1.0f, 0.842f, 0.227f, 1.0f };

		colors[ImGuiCol_TabDimmed]				= ImVec4{ 0.141f, 0.141f, 0.141f, 1.0f };
		colors[ImGuiCol_TabDimmedSelected]		= ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };

		// Resize
		colors[ImGuiCol_ResizeGrip]				= ImVec4{ 1.0f, 0.842f, 0.227f, 0.50f };
		colors[ImGuiCol_ResizeGripHovered]		= ImVec4{ 1.0f, 0.842f, 0.227f, 0.67f };
		colors[ImGuiCol_ResizeGripActive]		= ImVec4{ 1.0f, 0.842f, 0.227f, 0.95f };

		// Title
		colors[ImGuiCol_TitleBg]				= ImVec4{ 0.157f, 0.157f, 0.157f, 1.0f };
		colors[ImGuiCol_TitleBgActive]			= colors[ImGuiCol_TitleBg];
		colors[ImGuiCol_TitleBgCollapsed]		= colors[ImGuiCol_TitleBg];

		// Drag and Drop
		colors[ImGuiCol_DragDropTarget]			= ImVec4{ 1.0f, 0.842f, 0.227f, 0.416f };
	}

	uint32_t GuiLayer::GetActiveWidgetID() const
	{
		return GImGui->ActiveId;
	}

}