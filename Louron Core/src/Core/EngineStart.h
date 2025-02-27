#pragma once

// Louron Core Headers
#include "Engine.h"
#include "Logging.h"

// C++ Standard Library Headers

// External Vendor Library Headers

extern Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args);

namespace Louron {

	int Main_Entry(int argc, char** argv)
	{
		Louron::LoggingSystem::Init();

		auto app = CreateEngine({ argc, argv });

		if (!app)
			return -1;

		app->Run();

		delete app;

		return 0;
	}
}

#if _DEBUG

#pragma comment(linker, "/subsystem:console")
int main(int argc, char** argv) {

	return Louron::Main_Entry(argc, argv);
}

#else

#include <Windows.h>
#pragma comment(linker, "/subsystem:windows")
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) 
{
	return Louron::Main_Entry(__argc, __argv);
}

#endif