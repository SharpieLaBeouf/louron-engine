#pragma once

//#include <entt/entt.hpp>

#include "Entity.h"

class Scene {

public:

	Scene() = default;
	Scene(const Scene&) = default;
	~Scene() = default;

	bool LoadScene(const char* sceneFile) { return true; }

private:
	//entt::registry m_Registry;

	friend class Entity;
};