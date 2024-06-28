#pragma once

#define PX_RELEASE(x)			if(x)	{ x->release(); x = NULL;	}

// Louron Core Headers

// C++ Standard Library Headers

// External Vendor Library Headers
#include <PxPhysicsAPI.h>

namespace Louron {

	class Physics {

	public:

		static bool Init();

	private:

		// Delete default constructor
		Physics() = delete;

		// Delete copy assignment and move assignment constructors
		Physics(const Physics&) = delete;
		Physics(Physics&&) = delete;

		// Delete copy assignment and move assignment operators
		Physics& operator=(const Physics&) = delete;
		Physics& operator=(Physics&&) = delete;

		static physx::PxDefaultAllocator mAllocator;
		static physx::PxDefaultErrorCallback mErrorCallback;
		static physx::PxFoundation* mFoundation;
		static physx::PxPhysics* mPhysics ;
		static physx::PxDefaultCpuDispatcher* mDispatcher;
		static physx::PxPvd* mPvd;

	};

}