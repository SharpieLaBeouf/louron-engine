#pragma once

#include <iostream>
#include <stack>

#include "../../include/imgui/imgui.h"

#include "../../Headers/Input.h"
#include "../../Headers/Scene/Entity.h"
#include "../../Headers/Camera.h"
#include "../../Headers/Scene/InstanceManager.h"
#include "../../Headers/Abstracted GL/Light.h"
#include "../../Headers/Abstracted GL/Shader.h"
#include "../../Headers/Abstracted GL/Texture.h"
#include "../../Headers/Abstracted GL/Material.h"

/* 
  
  |-> This file is to demonstrate what an optimal 
  |   scene will look like in code. 
  |   

  |-> The constructor will load the environment with all relevant 
  |   entities and sub components required to run a scene from a 
  |   custom scene file.
  
  |-> The update loop will control game logic by calling 
  |   script components relevant update functions to control 
  |   entity properties throughout the current environment.
     
  |-> The draw loop will submit all renderable objects to the
  |   renderer required for rendering, determined by an 
  |   algorithm which tests the current state of the frame.
  
*/

namespace State {
	class OptimalScene : public State {

		//Private Setup Variables
	private:

		InstanceManager* m_InstanceManager;

		Window* m_Window;
		InputManager* m_Input;
		ShaderLibrary* m_ShaderLib;
		TextureLibrary* m_TextureLib;

	public:


	};
}