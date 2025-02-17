#pragma once

// Louron Core Headers
#include "Window.h"
#include "GuiLayer.h"
#include "LayerStack.h"
#include "Audio.h"
#include "Logging.h"

#include "../OpenGL/Shader.h"
#include "../OpenGL/Texture.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers
#include <glad/glad.h>
#include <glfw/glfw3.h>

int main(int argc, char** argv); 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);

namespace Louron {

	struct EngineCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			return Args[index];
		}
	};

	struct EngineConfig
	{
		std::string Name = "Louron Engine";
		std::string WorkingDirectory;
		EngineCommandLineArgs CommandLineArgs;

	};

	class Engine {
	public:
		Engine(const EngineConfig& specification);
		virtual ~Engine() = default;

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& GetWindow() { return *m_Window; }
		GuiLayer* GetImGuiLayer() { return m_GuiLayer; }

		InputManager& GetInput() { return *m_Input; }

		static Engine& Get() { return *s_Instance; }
		void Close();

		const EngineConfig& GetSpecification() const { return m_Specification; }

		void SubmitToMainThread(const std::function<void()>& function);

	private:

		void Run();
		bool OnWindowClose();
		bool OnWindowResize();

		void ExecuteMainThreadQueue();

		std::vector<std::string> FindFilePaths(const std::string& extension);

	private:

		bool m_Running = true;
		bool m_Minimized = false;

		float m_FixedUpdateTimer = 0.0f;

		std::unique_ptr <Window> m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;
		EngineConfig m_Specification;

		// Resource Management Systems
		std::unique_ptr<InputManager> m_Input;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

	private:
		static Engine* s_Instance;
		friend int ::main(int argc, char** argv);
		friend int WINAPI ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
	};

	Engine* CreateEngine(EngineCommandLineArgs args);
}