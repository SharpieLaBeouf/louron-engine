#include "Query.h"

#include "../Core/Logging.h"

namespace Louron
{
	Query::Query()
	{
		glGenQueries(1, &m_QueryObject);
	}

	Query::Query(const Type& query_type)
	{
		glGenQueries(1, &m_QueryObject);
	}

	Query::~Query()
	{
		glDeleteQueries(1, &m_QueryObject);
	}

	Query::Query(const Query& other)
	{
		glGenQueries(1, &m_QueryObject);

		m_QueryType = other.m_QueryType;
		m_ProcessingQuery = other.m_ProcessingQuery;
		m_LastResult = other.m_LastResult;
	}

	Query::Query(Query&& other) noexcept
	{
		m_QueryType = other.m_QueryType; other.m_QueryType = Type::AnySamplesPassed;
		m_QueryObject = other.m_QueryObject; other.m_QueryObject = -1;
		m_ProcessingQuery = other.m_ProcessingQuery; other.m_ProcessingQuery = false;
		m_LastResult = other.m_LastResult; other.m_LastResult = 0;
	}

	Query& Query::operator=(const Query& other)
	{
		if (this == &other)
			return *this;

		glGenQueries(1, &m_QueryObject);

		m_QueryType = other.m_QueryType;
		m_ProcessingQuery = other.m_ProcessingQuery;
		m_LastResult = other.m_LastResult;

		return *this;
	}

	Query& Query::operator=(Query&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_QueryType = other.m_QueryType; other.m_QueryType = Type::AnySamplesPassed;
		m_QueryObject = other.m_QueryObject; other.m_QueryObject = -1;
		m_ProcessingQuery = other.m_ProcessingQuery; other.m_ProcessingQuery = false;
		m_LastResult = other.m_LastResult; other.m_LastResult = 0;

		return *this;
	}

	void Query::Begin() 
	{
		glBeginQuery(static_cast<GLenum>(m_QueryType), m_QueryObject);
		m_ProcessingQuery = true;
	}

	void Query::End()
	{
		glEndQuery(static_cast<GLenum>(m_QueryType));
		m_ProcessingQuery = true;
	}

	bool Query::IsProcessing() const
	{
		return m_ProcessingQuery;
	}

	bool Query::IsResultAvailable() const
	{
		if (!m_ProcessingQuery)
			return true;

		GLint result = 0;
		glGetQueryObjectiv(m_QueryObject, GL_QUERY_RESULT_AVAILABLE, &result);
		return result != GL_FALSE;
	}

	const uint32_t& Query::GetResult()
	{
		if (!m_ProcessingQuery)
			return m_LastResult;

		GLint result = -1;
		glGetQueryObjectiv(m_QueryObject, GL_QUERY_RESULT, &result);
		m_LastResult = static_cast<uint32_t>(result);

		m_ProcessingQuery = false;
		return m_LastResult;
	}

	const uint32_t& Query::GetLastCompleteResult() const
	{
		return m_LastResult;
	}

}
