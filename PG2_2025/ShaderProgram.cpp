#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

// set uniform according to name 
// https://docs.gl/gl4/glUniform

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
	std::vector<GLuint> shader_ids;

	// compile shaders and store IDs for linker
	shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
	shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

	// link all compiled shaders into shader_program 
	ID = link_shader(shader_ids);
}

void ShaderProgram::setUniform(const std::string& name, const float val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const int val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4 in_vec4) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform4fv(loc, 1, glm::value_ptr(in_vec4));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::getShaderInfoLog(const GLuint obj) {
	GLint log_length = 0;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 0) {
		std::vector<char> log(log_length);
		glGetShaderInfoLog(obj, log_length, nullptr, log.data());
		return std::string(log.begin(), log.end());
	}
	return "";

}

std::string ShaderProgram::getProgramInfoLog(const GLuint obj) {
	GLint log_length = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 0) {
		std::vector<char> log(log_length);
		glGetProgramInfoLog(obj, log_length, nullptr, log.data());
		return std::string(log.begin(), log.end());
	}
	return "";
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, const GLenum type) {
	GLuint shader_h;

	// TODO: implement, try to compile, check for error; if any, print compiler result (or print allways, if you want to see warnings as well)
	// if err, throw error
	std::string source = textFileRead(source_file);
	const char* source_cstr = source.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source_cstr, nullptr);
	glCompileShader(shader);

	// Check for errors
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		std::cerr << "Error compiling shader: " << source_file << "\n";
		std::cerr << getShaderInfoLog(shader) << std::endl;
		glDeleteShader(shader);
		throw std::runtime_error("Shader compilation failed.");
	}

	return shader;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
	GLuint prog = glCreateProgram();

	for (GLuint id : shader_ids)
		glAttachShader(prog, id);

	glLinkProgram(prog);

	// Check for errors
	GLint success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		std::cerr << "Error linking shader program\n";
		std::cerr << getProgramInfoLog(prog) << std::endl;
		glDeleteProgram(prog);
		throw std::runtime_error("Shader linking failed.");
	}

	// Cleanup: Detach and delete shaders after linking
	for (GLuint id : shader_ids) {
		glDetachShader(prog, id);
		glDeleteShader(id);
	}
	return prog;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filename) {
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error(std::string("Error opening file: ") + filename.string());
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}