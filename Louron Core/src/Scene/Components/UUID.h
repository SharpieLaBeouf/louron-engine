#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <cstdint>
#include <cstddef>

// External Vendor Library Headers


#define NULL_UUID UINT64_MAX

namespace Louron {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;

		uint64_t GenerateUUID();

	};

}

// Define specialisation for the hash template when custom type used (Louron::UUID)
namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Louron::UUID>
	{
		std::size_t operator()(const Louron::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}