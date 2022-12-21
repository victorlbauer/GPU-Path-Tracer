#include <PT.h>
#include "Shader.h"

namespace PT
{
	// Standard shader
	void Shader::Use() const
	{
		glUseProgram(m_id);
	}

	uint32_t Shader::GetProgram() const
	{
		return m_id;
	}

	void Shader::SetUniformBool(const std::string&& name, const bool& value) const
	{
		glUniform1i(glGetUniformLocation(m_id, name.c_str()), (int)value);
	}

	void Shader::SetUniformInt(const std::string&& name, const int& value) const
	{
		glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
	}

	void Shader::SetUniformUInt(const std::string&& name, const uint32_t& value) const
	{
		glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
	}

	void Shader::SetUniformFloat(const std::string&& name, const float& value) const
	{
		glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
	}

	void Shader::SetUniformVec3(const std::string&& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
	}

	void Shader::SetUniformMat4(const std::string&& name, const glm::mat4x4& value) const
	{
		glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	std::string Shader::ReadFile(const std::string& path)
	{
		m_path = path;
		std::string code;
		std::ifstream fileStream;

		try
		{
			fileStream.open(path);
			std::string line;
			// Read line by line
			while (std::getline(fileStream, line))
			{
				// Found an include
				if (line.find(m_include) != std::string::npos)
				{
					std::string includePath = line.substr(m_include.length());
					includePath.erase(std::remove(includePath.begin(), includePath.end(), '\"'), includePath.end());
					std::string subCode = ReadFile(path.substr(0, path.rfind("/")) + "/" + includePath);
					code.append(subCode);
				}
				else
				{
					code.append(line + "\n");
				}
			}
			fileStream.close();
		}
		catch (std::ifstream::failure)
		{
			LOG_WARNING("Failed to read shader file at: " + path + "\n");
		}

		return code;
	}

	void Shader::CheckCompilationErrors(const uint32_t&& shader) const
	{
		int success;
		char errlog[1024];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, errlog);
			LOG_WARNING(typeid(*this).name(), " compilation error!\nFile: ", m_path, "\n", errlog);
		}
	}

	void Shader::CheckLinkingErrors() const
	{
		int success;
		char errlog[1024];
		glGetProgramiv(m_id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(m_id, 1024, NULL, errlog);
			LOG_WARNING(typeid(*this).name(), " linking error!\nFile: ", m_path, "\n", errlog, "\n");
		}
	}

	// Compute shader
	ComputeShader::ComputeShader(const std::string&& path)
	{
		ComputeShaderProgram(std::forward<const std::string&&>(path));
	}

	void ComputeShader::ComputeShaderProgram(const std::string&& path)
	{
		std::string code = ReadFile(std::forward<const std::string>(path));
		const char* shaderCode = code.c_str();

		// Compile
		uint32_t shader;
		shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &shaderCode, NULL);
		glCompileShader(shader);
		CheckCompilationErrors(std::forward<uint32_t>(shader));

		// Link
		m_id = glCreateProgram();
		glAttachShader(m_id, shader);
		glLinkProgram(m_id);
		CheckLinkingErrors();

		glDeleteShader(shader);
	}

	void ComputeShader::Dispatch(const uint32_t& x, const uint32_t& y, const uint32_t& z)
	{
		glDispatchCompute(x, y, z);
	}

	void ComputeShader::Barrier(uint32_t&& barrierBit)
	{
		glMemoryBarrier(barrierBit);
	}

	// Pixel shader
	PixelShader::PixelShader(const std::string&& path)
	{
		PixelShaderProgram(std::forward<const std::string&&>(path));
	}

	void PixelShader::PixelShaderProgram(const std::string&& path)
	{
		std::string code = ReadFile(std::forward<const std::string>(path));

		// Vertex
		std::string compilingVertex = code;
		compilingVertex.insert(m_version.length() + 1, m_defVertex);
		const char* vertexCode = compilingVertex.c_str();

		// Fragment
		std::string compilingFragment = code;
		compilingFragment.insert(m_version.length() + 1, m_defFragment);
		const char* fragmentCode = compilingFragment.c_str();

		uint32_t vertex, fragment;
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertexCode, NULL);
		glCompileShader(vertex);
		CheckCompilationErrors(std::forward<uint32_t>(vertex));

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentCode, NULL);
		glCompileShader(fragment);
		CheckCompilationErrors(std::forward<uint32_t>(fragment));

		m_id = glCreateProgram();
		glAttachShader(m_id, vertex);
		glAttachShader(m_id, fragment);
		glLinkProgram(m_id);
		CheckLinkingErrors();

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
}