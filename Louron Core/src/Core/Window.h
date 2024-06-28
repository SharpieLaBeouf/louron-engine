#pragma once

// Louron Core Headers
#include "Input.h"

// C++ Standard Library Headers
#include <iostream>

// External Vendor Library Headers
#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace Louron {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Louron Engine",
			uint32_t width = 1600,
			uint32_t height = 900)
			: Title(title), Width(width), Height(height) {	}
	};

	class Window {

	public:
		Window(const WindowProps& props);
		~Window();

		void OnUpdate();

		unsigned int GetWidth() const { return m_Data->Width == 0 ? 1 : m_Data->Width; }
		unsigned int GetHeight() const { return m_Data->Height == 0 ? 1 : m_Data->Height; }

		glm::uvec2 GetSize() const { return { m_Data->Width == 0 ? 1 : m_Data->Width, m_Data->Height == 0 ? 1 : m_Data->Height }; }

		void* GetNativeWindow() const { return m_Window; }

		void SetVSync(bool enabled);

		static std::unique_ptr<Window> Create(const WindowProps& props = WindowProps());

	private:

		void Init(const WindowProps& props);
		void Shutdown();

	private:

		struct WindowData {
			std::string Title;
			uint32_t Width = 1600, Height = 800;
			bool VSync;
		};

	public:

		WindowData& GetData() const { return *m_Data; }

	private:

		GLFWwindow* m_Window;

		std::unique_ptr<WindowData> m_Data;

		//friend static void windowSizeCallBack(GLFWwindow* window, int w, int h);
	};
}