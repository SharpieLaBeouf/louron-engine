// CUSTOM HEADERS
#include "Test Scene Application.h"

TestSceneApplication game;

int main()
{
	if (!game.run())
		return -1;

	return 0;
}