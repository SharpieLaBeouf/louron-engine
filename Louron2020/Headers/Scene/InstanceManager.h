#pragma once

#include "../Window.h"
#include "../Input.h"

#include "../Abstracted GL/Shader.h"
#include "../Abstracted GL/Texture.h"

class State {

public:
	virtual void update() = 0;
	virtual void draw() = 0;

	virtual ~State() = default;

private:

};

class InstanceManager {
	
public:

	InstanceManager(Window* wnd, InputManager* inp, ShaderLibrary* shaderLib, TextureLibrary* texLib, std::vector<std::unique_ptr<State>>* states)
		: m_Window(wnd), m_Input(inp), m_ShaderLib(shaderLib), m_TextureLib(texLib), m_States(states) { }
	~InstanceManager() = default;

	Window* getWindowInstance() { return m_Window; }
	InputManager* getInputInstance() { return m_Input; }
	ShaderLibrary* getShaderLibInstance() { return m_ShaderLib; }
	TextureLibrary* getTextureLibInstance() { return m_TextureLib; }

	std::vector<std::unique_ptr<State>>* getStatesInstance() { return m_States; }

private:

	Window* m_Window;
	InputManager* m_Input;
	ShaderLibrary* m_ShaderLib;
	TextureLibrary* m_TextureLib;
	std::vector<std::unique_ptr<State>>* m_States;
};