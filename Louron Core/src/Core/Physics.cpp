#include "Physics.h"

#include "Logging.h"
#include <iostream>

namespace Louron {

	// Define static member variables
	physx::PxDefaultAllocator Physics::mAllocator;
	physx::PxDefaultErrorCallback Physics::mErrorCallback;
	physx::PxFoundation* Physics::mFoundation = nullptr;
	physx::PxPhysics* Physics::mPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* Physics::mDispatcher = nullptr;
	physx::PxPvd* Physics::mPvd = nullptr;

	bool Physics::Init() {

		// 1. Init Physics Founation
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
		if (!mFoundation) {
			L_CORE_FATAL("PxCreateFoundation Failed!");
			return false;
		}
		else {
			L_CORE_INFO("PxCreateFoundation Successful!");

			// 2. Init PhysX Visual Debugger (PVD) if in debug
#ifdef _DEBUG
			mPvd = physx::PxCreatePvd(*mFoundation);
			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
			mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
#endif

			// 3. Init top level physics object
			mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, mPvd);

			if (!mPhysics) {
				L_CORE_FATAL("PxCreatePhysics Failed!");

				if (mPvd)
				{
					physx::PxPvdTransport* transport = mPvd->getTransport();
					mPvd->release();
					mPvd = nullptr;
					PX_RELEASE(transport);
				}

				PX_RELEASE(mFoundation);
				mFoundation = nullptr;

				return false; // Initialization failed
			}
			else {
				L_CORE_INFO("PxCreatePhysics Successful!");

				// 4. Sets up default CPU dispatcher for the PhysX simulation
				mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

				return true; // Initialization succeeded
			}
		}
	}
}

