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
		std::string Name = "Louron Engine";
		std::string WorkingDirectory;
		EngineCommandLineArgs CommandLineArgs;

	};

	class VertexArray;
	struct RenderFBO {

		GLuint FBO = -1;
		std::unique_ptr<Texture> RenderTexture = nullptr;
		std::unique_ptr<VertexArray> ScreenQuadVAO = nullptr;

		void Init();
	};

	class Engine {
	public:
		Engine(const EngineSpecification& specification);
		virtual ~Engine() = default;

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& GetWindow() { return *m_Window; }
		GuiLayer* GetImGuiLayer() { return m_GuiLayer; }

		InputManager& GetInput() { return *m_Input; }
		ShaderLibrary& GetShaderLibrary() { return *m_ShaderLibrary; }
		TextureLibrary& GetTextureLibrary() { return *m_TextureLibrary; }

		RenderFBO& GetRenderFBO() { return m_RenderFBO; }

		static Engine& Get() { return *s_Instance; }
		void Close();

		const EngineSpecification& GetSpecification() const { return m_Specification; }

	private:

		void Run();
		bool OnWindowClose();
		bool OnWindowResize();

		std::vector<std::string> FindFilePaths(const std::string& directory, const std::string& extension);

	private:

		bool m_Running = true;
		bool m_Minimized = false;

		float m_FixedUpdateTimer = 0.0f;

		RenderFBO m_RenderFBO;

		std::unique_ptr <Window> m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;
		EngineSpecification m_Specification;

		// Resource Management Systems
		std::unique_ptr<InputManager> m_Input;
		std::unique_ptr<ShaderLibrary> m_ShaderLibrary;
		std::unique_ptr<TextureLibrary> m_TextureLibrary;

	private:
		static Engine* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	Engine* CreateEngine(EngineCommandLineArgs args);
}