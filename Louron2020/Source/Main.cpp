// WINDOWS LIBRARIES
#include <iostream>
#include <conio.h>

// CUSTOM HEADERS
#include "../Headers/Engine.h"


Engine game;

int main()
{
#ifndef _DEBUG
#if defined _WIN64 || defined _WIN32
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
#endif Hide Console on Release

	game.run();

	return 0;
}