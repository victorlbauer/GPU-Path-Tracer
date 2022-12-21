#include <PT.h>
#include "Texture.h"

namespace PT
{
	Texture::Texture() : m_id(0), m_width(0), m_height(0) {}

	Texture::Texture(const std::string&& path) : m_id(0), m_width(0), m_height(0)
	{
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);	

		if (data)
		{
			m_width = width;
			m_height = height;
			glBindTexture(GL_TEXTURE_2D, m_id);
			glGenTextures(1, &m_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, data);
			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);
		}
		else
			LOG_WARNING("Failed to load texture at: ", path, "\n");
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_id);
	}

	void Texture::LoadData(uint32_t&& internalFormat, GLenum&& format, GLenum&& type, const void* data) const
	{
		Bind();
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, type, data);
		Unbind();
	}
	
	void Texture::BindTextureUnit(uint32_t&& internalFormat, GLenum&& access, uint32_t&& unit) const
	{
		Bind();
		glBindImageTexture(unit, m_id, 0, GL_FALSE, 0, access, internalFormat);
		Unbind();
	}

	void Texture::ActiveTexture(uint32_t&& unit) const
	{
		glActiveTexture(GL_TEXTURE0 + unit);
	}

	void Image::Init(const uint32_t& width, const uint32_t& height)
	{
		m_width = width;
		m_height = height;

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}