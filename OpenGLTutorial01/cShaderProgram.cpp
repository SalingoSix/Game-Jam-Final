#include "cShaderProgram.h"

cShaderProgram::cShaderProgram()
{

}

cShaderProgram::~cShaderProgram()
{

}

void cShaderProgram::compileProgram(std::string path, std::string vertFile, std::string fragFile, std::string geomFile)
{
	vertexShader.setPath(path);
	vertexShader.readFile(vertFile);

	vertexShader.ID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader.ID, vertexShader.numberOfLines, vertexShader.arraySource, NULL);
	glCompileShader(vertexShader.ID);

	int success;
	glGetShaderiv(vertexShader.ID, GL_COMPILE_STATUS, &success);

	char infoLog[512];
	if (!success)
	{
		glGetShaderInfoLog(vertexShader.ID, 512, NULL, infoLog);
		std::cout << "Vertex Shader compilation failed:\n" << infoLog << std::endl;
	}

	fragmentShader.setPath(path);
	fragmentShader.readFile(fragFile);

	fragmentShader.ID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader.ID, fragmentShader.numberOfLines, fragmentShader.arraySource, NULL);
	glCompileShader(fragmentShader.ID);

	glGetShaderiv(fragmentShader.ID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader.ID, 512, NULL, infoLog);
		std::cout << "Fragment Shader compilation failed:\n" << infoLog << std::endl;
	}

	if (geomFile != "")
	{
		geometryShader.setPath(path);
		geometryShader.readFile(geomFile);

		geometryShader.ID = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader.ID, geometryShader.numberOfLines, geometryShader.arraySource, NULL);
		glCompileShader(geometryShader.ID);

		glGetShaderiv(geometryShader.ID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometryShader.ID, 1024, NULL, infoLog);
			std::cout << "Geometry Shader compilation failed:\n" << infoLog << std::endl;
		}
	}






	this->ID = glCreateProgram();

	glAttachShader(this->ID, vertexShader.ID);
	glAttachShader(this->ID, fragmentShader.ID);
	glAttachShader(this->ID, geometryShader.ID);
	glLinkProgram(this->ID);

	glDeleteShader(vertexShader.ID);
	glDeleteShader(fragmentShader.ID);
	glDeleteShader(geometryShader.ID);
}

void cShaderProgram::useProgram()
{
	glUseProgram(this->ID);
}

void cShaderProgram::setBool(std::string name, bool value)
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void cShaderProgram::setInt(std::string name, int value)
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void cShaderProgram::setFloat(std::string name, float value)
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void cShaderProgram::setVec3(std::string name, glm::vec3 value)
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}

void cShaderProgram::setVec3(std::string name, float x, float y, float z)
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void cShaderProgram::SetVec4(std::string name, glm::vec4 value)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}

void cShaderProgram::setMat4(std::string name, glm::mat4 value)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void cShaderProgram::setMat4(std::string name, int count, glm::mat4* value)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), count, GL_FALSE, (const GLfloat*) glm::value_ptr(*value));
}