#pragma once

#include "../Headers/Window.h"

namespace State {

	class SceneState {

	public:
		virtual void update(Window* wnd) = 0;
		virtual void draw(Window* wnd) = 0;

		virtual ~SceneState() { };
	};

}