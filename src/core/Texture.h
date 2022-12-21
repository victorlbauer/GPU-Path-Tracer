#pragma once
#include "Logger.h"

namespace PT
{
	class Texture
	{
		public:
			Texture();
			explicit Texture(const std::string&& path);
			virtual ~Texture();
	
			virtual void LoadData(uint32_t&& internalFormat, GLenum&& format, GLenum&& type, const void* data) const;
			virtual void BindTextureUnit(uint32_t&& internalFormat, GLenum&& access, uint32_t&& unit) const;
			virtual void ActiveTexture(uint32_t&& unit) const;

			inline virtual void Bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }
			inline virtual void Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

			inline virtual const uint32_t& GetTexture() const { return m_id; }
			inline virtual const uint32_t& GetWidth()	const { return m_width; }
			inline virtual const uint32_t& GetHeight()	const { return m_height; }

		protected:
			uint32_t m_id;
			uint32_t m_width;
			uint32_t m_height;
	};

	class Image : public Texture
	{
		public:
			explicit Image() = default;
			void Init(const uint32_t& width, const uint32_t& height);
	};
}