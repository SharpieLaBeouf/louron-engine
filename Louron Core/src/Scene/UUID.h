#pragma once

#include <cstdint>
#include <cstddef>

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