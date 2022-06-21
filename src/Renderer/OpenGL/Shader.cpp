#include "Ren/Renderer/OpenGL/Shader.h"
#include "Ren/Core.h"
#include <glad/glad.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>

using namespace Ren;

const Shader& Shader::Use() const
{
	glUseProgram(ID);
	return *this;
}

void Shader::Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource)
{
	unsigned int sVertex, sFragment, sGeom;

	sVertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(sVertex, 1, &vertexSource, NULL);
	glCompileShader(sVertex);
	checkCompileErrors(sVertex, "VERTEX");

	sFragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sFragment, 1, &fragmentSource, NULL);
	glCompileShader(sFragment);
	checkCompileErrors(sFragment, "FRAGMENT");

	if (geometrySource != nullptr)
	{
		sGeom = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(sGeom, 1, &geometrySource, NULL);
		glCompileShader(sGeom);
		checkCompileErrors(sGeom, "GEOMETRY");
	}

	this->ID = glCreateProgram();
	glAttachShader(this->ID, sVertex);
	glAttachShader(this->ID, sFragment);
	if (geometrySource != nullptr)
		glAttachShader(this->ID, sGeom);
	glLinkProgram(this->ID);
	checkCompileErrors(this->ID, "PROGRAM");

	glDeleteShader(sVertex);
	glDeleteShader(sFragment);
	if (geometrySource != nullptr)
		glDeleteShader(sGeom);
}

void Shader::SetFloat(const char* name, float value, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform1f(glGetUniformLocation(this->ID, name), value);
}
void Shader::SetInt(const char* name, int value, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform1i(glGetUniformLocation(this->ID, name), value);
}
void Shader::SetVec2f(const char* name, float x, float y, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform2f(glGetUniformLocation(this->ID, name), x, y);
}
void Shader::SetVec2f(const char* name, const glm::vec2& value, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
}
void Shader::SetVec3f(const char* name, float x, float y, float z, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
}
void Shader::SetVec3f(const char* name, const glm::vec3& value, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
}
void Shader::SetVec4f(const char* name, float x, float y, float z, float w, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
}
void Shader::SetVec4f(const char* name, const glm::vec4& value, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
}
void Shader::SetMat4(const char* name, const glm::mat4& matrix, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
}
void Shader::SetMat3(const char* name, const glm::mat3& matrix, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniformMatrix3fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
}
void Shader::SetVec2i(const char* name, int x, int y, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform2i(glGetUniformLocation(this->ID, name), x, y);
}
void Shader::SetVec2i(const char* name, const glm::ivec2& value, bool useShader) const
{
	if (useShader)
		this->Use();
	glUniform2i(glGetUniformLocation(this->ID, name), value.x, value.y);
}

void Shader::checkCompileErrors(unsigned int object, std::string type)
{
	int compile_success = 0;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(object, GL_COMPILE_STATUS, &compile_success);
		if (!compile_success)
		{
			glGetShaderInfoLog(object, 1024, NULL, infoLog);
			Logger::LogE("[SHADER]: Compile-time error: Type: " + type + "\n" + infoLog, "", -1);
		}
		REN_ASSERT(compile_success, "Shader compilation failed.");
	}
	else
	{
		glGetProgramiv(object, GL_LINK_STATUS, &compile_success);
		if (!compile_success)
		{
			glGetProgramInfoLog(object, 1024, NULL, infoLog);
			Logger::LogE("[SHADER]: Link-time error: Type: " + type + "\n" + infoLog, "", -1);
		}
		REN_ASSERT(compile_success, "Shader compilation failed.");
	}
}

Shader Shader::LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try
	{
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);

		REN_ASSERT(vertexShaderFile.is_open(), "Cannot open vertex shader file '" + std::string(vShaderFile) + "'.");
		REN_ASSERT(fragmentShaderFile.is_open(), "Cannot open fragment shader file '" + std::string(fShaderFile) + "'.");

		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();

		vertexShaderFile.close();		
		fragmentShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		if (gShaderFile != nullptr)
		{
			std::ifstream geometryShaderFile(gShaderFile);
			REN_ASSERT(geometryShaderFile.is_open(), "Cannot open geometry shader file '" + std::string(gShaderFile) + "'.");
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception& e)
	{
		throw std::runtime_error(("Failed to read shader files\n" + std::string(e.what())).c_str());
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();

	Shader shader;
	shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
	return shader;
}
Shader Shader::LoadShaderFromFile(const char* filename_glsl)
{
    std::ifstream ifs(filename_glsl);
    REN_ASSERT(ifs.is_open(), "Cannot open shader file '" + std::string(filename_glsl) + "'");

    std::stringstream stream;
    stream << ifs.rdbuf();

	// Split file into individual parts with '@' as a delimiter character.
	// Like this, the word witch was after '@' character, will be on first line.
    std::vector<std::string> parts;
    std::string str;
    while (std::getline(stream, str, '@'))
        if (!str.empty() && str.find("#version") != std::string::npos)
            parts.push_back(str);
    

	// Assign inidivual parts to the sources. Also remove the first line, which identifies the shader type.
    const char* vertex_source = nullptr;
    const char* geometry_source = nullptr;
    const char* fragment_source = nullptr;
    for (auto&& part : parts)
    {
        std::string first_line = part.substr(0, part.find_first_of('\n') + 1);

        if (first_line.find("vertex") != std::string::npos)
            vertex_source = part.erase(0, part.find_first_of('\n') + 1).c_str();
        else if (first_line.find("geometry") != std::string::npos)
            geometry_source = part.erase(0, part.find_first_of('\n') + 1).c_str();
        else if (first_line.find("fragment") != std::string::npos)
            fragment_source = part.erase(0, part.find_first_of('\n') + 1).c_str();
    }

	Shader shader;
	shader.Compile(vertex_source, fragment_source, geometry_source);
	return shader;
}	