#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <cstdint>
#include <cstddef>

// External Vendor Library Headers


#define NULL_UUID UINT32_MAX

namespace Louron {

	class UUID
	{
	public:
		UUID();
		UUID(uint32_t uuid);
		UUID(const UUID&) = default;

		uint32_t GetUUID() const { return m_UUID; }

		operator uint32_t() const { return m_UUID; }
	private:
		uint32_t m_UUID;

		uint32_t GenerateUUID();

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
			return (uint32_t)uuid;
		}
	};

}