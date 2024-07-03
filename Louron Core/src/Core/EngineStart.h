#pragma once

// Louron Core Headers
#include "Engine.h"
#include "Logging.h"

// C++ Standard Library Headers

// External Vendor Library Headers

extern Louron::Engine* Louron::CreateEngine(Louron::EngineCommandLineArgs args);

#ifdef _DEBUG

#pragma comment( linker, "/subsystem:console" )
int main(int argc, char** argv) {

	Louron::LoggingSystem::Init();

	auto app = Louron::CreateEngine({ argc, argv });

#else

#include <Windows.h>
#pragma comment( linker, "/subsystem:windows" )
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	Louron::LoggingSystem::Init();

	auto app = Louron::CreateEngine({});

#endif


	app->Run();

	delete app;


	return 0;
}