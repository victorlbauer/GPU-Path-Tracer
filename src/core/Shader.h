#pragma once
#include "Logger.h"

namespace PT
{
	class Shader
	{
		public:
			virtual ~Shader() = default;

			void Use() const;
			uint32_t GetProgram() const;

			void SetUniformBool (const std::string&& name, const bool& value) const;
			void SetUniformInt	(const std::string&& name, const int& value) const;
			void SetUniformUInt (const std::string&& name, const uint32_t& value) const;
			void SetUniformFloat(const std::string&& name, const float& value) const;
			void SetUniformVec3 (const std::string&& name, const glm::vec3& value) const;
			void SetUniformMat4 (const std::string&& name, const glm::mat4x4& value) const;

		protected:
			explicit Shader() : m_id(0) {}

			std::string ReadFile(const std::string& path);
			void CheckCompilationErrors(const uint32_t&& shader) const;
			void CheckLinkingErrors() const;

		protected:
			uint32_t m_id;
			std::string m_path;
			const std::string m_include = "#include ";

	};

	class ComputeShader : public Shader
	{
		public:
			explicit ComputeShader() = default;
			explicit ComputeShader(const std::string&& path);
			void ComputeShaderProgram(const std::string&& path);
			void Dispatch(const uint32_t& x, const uint32_t& y, const uint32_t& z);
			void Barrier(uint32_t&& barrierBit);

	};

	class PixelShader : public Shader
	{
		public:
			explicit PixelShader() = default;
			explicit PixelShader(const std::string&& path);
			void PixelShaderProgram(const std::string&& path);

		private:
			const std::string m_version		= "#version 430 core";
			const std::string m_defVertex   = "#define COMPILING_VERTEX_SHADER\n";
			const std::string m_defFragment = "#define COMPILING_FRAGMENT_SHADER\n";
	};
}