#pragma once

#include "../Headers/Window.h"

namespace State {

	class SceneState {

	public:
		virtual void update() = 0;
		virtual void draw() = 0;

		virtual ~SceneState() { };
	};

}