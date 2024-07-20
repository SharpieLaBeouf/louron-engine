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
#include "Test Projects/Scene 11.h"
#include "Test Projects/Scene 12.h"
#include "Test Projects/Scene 13.h"
#include "Test Projects/Scene 14.h"
#include "Test Projects/Scene 15.h"
#include "Test Projects/Scene 16.h"
#include "Test Projects/Scene 17.h"

class MainMenuLayer : public Louron::Layer {
public:

	MainMenuLayer();
	virtual ~MainMenuLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate() override;
	virtual void OnFixedUpdate() override;
	virtual void OnGuiRender() override;

private:
	uint32_t m_SceneSelector = 0;
	std::vector<std::unique_ptr<TestScene>> m_Scenes;
};