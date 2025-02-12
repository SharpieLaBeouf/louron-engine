#pragma once

#include "glad/glad.h"

#include <cstdint>

namespace Louron {

	class Query
	{

	public:
		
		enum class Type
		{
			SamplesPassed = GL_SAMPLES_PASSED, // Returns number of samples passing depth test
			AnySamplesPassed = GL_ANY_SAMPLES_PASSED, // Returns true/false if any sample passed
			AnySamplesPassedConservative = GL_ANY_SAMPLES_PASSED_CONSERVATIVE // Returns true/false if any sample passed with more false positives
		};

		Query();
		Query(const Type& query_type);
		~Query();

		Query(const Query& other);
		Query(Query&& other) noexcept;

		Query& operator =(const Query& other);
		Query& operator =(Query&& other) noexcept;

		void Begin();
		void End();

		bool IsProcessing() const;
		bool IsResultAvailable() const;

		const uint32_t& GetResult();
		const uint32_t& GetLastCompleteResult() const;

	private:

		Type m_QueryType = Type::AnySamplesPassed;
		GLuint m_QueryObject = -1;
		bool m_ProcessingQuery = false;

		// Result Based on Type
		// Samples Passed - this will be number of samples passed
		// Any Samples Passed - this will be either 0 (FALSE), or 1 (TRUE)
		uint32_t m_LastResult = 0;
	};

}

