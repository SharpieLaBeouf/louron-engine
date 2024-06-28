#pragma once

// Louron Core Headers
#include "../Components/Components.h"

// C++ Standard Library Headers
#include <memory>

// External Vendor Library Headers


namespace Louron {

	class Scene;
	class Entity;

	class TransformSystem {

	public:

		static void Update(std::shared_ptr<Scene> scene);

	private:

		TransformSystem() = delete;
		TransformSystem(const TransformSystem&) = delete;
		~TransformSystem() = delete;

	};


}