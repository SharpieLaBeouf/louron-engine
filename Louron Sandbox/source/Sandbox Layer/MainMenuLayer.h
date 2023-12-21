#pragma once

#include <iostream>
#include <string>

#include "Louron.h"
#include "Core/Layer.h"

#include "Test Projects/Scene 1.h"
#include "Test Projects/Scene 2.h"
#include "Test Projects/Scene 3.h"
#include "Test Projects/Scene 4.h"
#include "Test Projects/Scene 5.h"
#include "Test Projects/Scene 6.h"
#include "Test Projects/Scene 7.h"
#include "Test Projects/Scene 8.h"
#include "Test Projects/Scene 9.h"

class MainMenuLayer : public Louron::Layer {
public:

	MainMenuLayer();
	virtual ~MainMenuLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate() override;
	virtual void OnGuiRender() override;

private:
	int m_SceneSelector = 0;
	std::unique_ptr<Scene1> scene1;
	std::unique_ptr<Scene2> scene2;
	std::unique_ptr<Scene3> scene3;
	std::unique_ptr<Scene4> scene4;
	std::unique_ptr<Scene5> scene5;
	std::unique_ptr<Scene6> scene6;
	std::unique_ptr<Scene7> scene7;
	std::unique_ptr<Scene8> scene8;
	std::unique_ptr<Scene9> scene9;
};