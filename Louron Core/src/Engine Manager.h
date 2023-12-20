#pragma once

// VENDOR LIBRARIES
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

// ENGINE SYSTEMS
#include "Core/Input.h"
#include "Core/Window.h"

// ENGINE OPENGL
#include "OpenGL/Shader.h"
#include "OpenGL/Texture.h"
#include "OpenGL/Material.h"
#include "OpenGL/Vertex Array.h"

namespace Louron {

	class LouronEngine {

	public:

		LouronEngine();
		~LouronEngine();

		int StartEngine();
		int EngineLoop();

		Window* getWindowInstance() { return m_WindowManager; }
		InputManager* getInputInstance() { return m_InputManager; }
		ShaderLibrary* getShaderLibInstance() { return m_ShaderLibrary; }
		TextureLibrary* getTextureLibInstance() { return m_TextureLibrary; }

	private:

		Window* m_WindowManager = nullptr;
		InputManager* m_InputManager = nullptr;
		ShaderLibrary* m_ShaderLibrary = nullptr;
		TextureLibrary* m_TextureLibrary = nullptr;
	};
}