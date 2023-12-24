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
#include "Test Projects/Scene 10.h"

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
	std::vector<std::unique_ptr<Scene>> m_Scenes;
};