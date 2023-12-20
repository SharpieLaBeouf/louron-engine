#pragma once

#include "Entity.h"

namespace Louron {

	class Scene {

	public:

		Scene() = default;
		Scene(const Scene&) = default;
		~Scene() = default;

		bool LoadScene(const char* sceneFile) { return true; }

	private:

		friend class Entity;
	};
}