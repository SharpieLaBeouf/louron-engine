#pragma once

#include "../Headers/Window.h"

namespace State {

	class GameState {

	public:
		virtual void update(Window* wnd) = 0;
		virtual void draw() = 0;

		virtual ~GameState() { };
	};

}