#include "UUID.h"

// Louron Core Headers

// C++ Standard Library Headers
#include <random>
#include <unordered_map>
#include <limits>

// External Vendor Library Headers

namespace Louron {

	static std::random_device s_RandomDevice;
	static std::mt19937 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint32_t> s_UniformDistribution;

	UUID::UUID()
		: m_UUID(s_UniformDistribution(s_Engine))
	{
	}

	UUID::UUID(uint32_t uuid)
		: m_UUID(uuid)
	{
	}

	uint32_t UUID::GenerateUUID() {
		std::uniform_int_distribution<uint32_t> distribution(0, std::numeric_limits<uint32_t>::max() - 1);
		return distribution(s_Engine);
	}

}