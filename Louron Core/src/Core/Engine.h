#pragma once

#include "LayerStack.h"
#include "Window.h"
#include "GuiLayer.h"

int main(int argc, char** argv);

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

	struct EngineSpecification
	{
		std::string Name = "Hazel Application";
		std::string WorkingDirectory;
		EngineCommandLineArgs CommandLineArgs;
	};

	class Engine {
	public:
		Engine(const EngineSpecification& specification);
		virtual ~Engine();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& GetWindow() { return m_Window; }

		void Close();

		GuiLayer* GetImGuiLayer() { return m_GuiLayer; }

		static Engine& Get() { return *s_Instance; }

		const EngineSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Run();
		bool OnWindowClose();
		bool OnWindowResize();

	private:
		Window m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;
		EngineSpecification m_Specification;

		bool m_Running = true;
		bool m_Minimized = false;
		float m_LastFrameTime = 0.0f;

	private:
		static Engine* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	Engine* CreateEngine(EngineCommandLineArgs args);
}