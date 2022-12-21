#include <PT.h>
#include "Buffer.h"

namespace PT
{
	VAO::~VAO()
	{
		glDeleteVertexArrays(1, &m_id);
	}

	void VAO::Init()
	{
		glGenVertexArrays(1, &m_id);
		glBindVertexArray(m_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VAO::Bind()
	{
		glBindVertexArray(m_id);
	}

	void VAO::Unbind()
	{
		glBindVertexArray(0);
	}

	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void GLBuffer::InitBuffer(const uint32_t& type, const uint32_t& storageType)
	{
		m_type = type;
		m_storageType = storageType;
		glGenBuffers(1, &m_id);
	}

	void GLBuffer::Bind()
	{
		glBindBuffer(m_type, m_id);
	}

	void GLBuffer::Unbind()
	{
		glBindBuffer(m_type, 0);
	}

	void GLBuffer::InitData(const size_t& size, const uint32_t&& n, const uint32_t&& bufferIndex)
	{
		Bind();
		glBufferData(m_type, size * n, nullptr, m_storageType);
		glBindBufferBase(m_type, bufferIndex, m_id);
		Unbind();
	}
}