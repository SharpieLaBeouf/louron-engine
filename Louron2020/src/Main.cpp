// WINDOWS LIBRARIES
#include <iostream>
#include <conio.h>

// CUSTOM HEADERS
#include "Application.h"

Application game;

int main()
{
	if (!game.run())
		return -1;

	return 0;
}