#pragma once

#include "glad/glad.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

void Shader(const GLchar* vertexPath, const GLchar* fragmentPath, unsigned int &Program);

void setVec3(unsigned int &program, const GLchar* name, glm::vec3 value);

void setMat4(unsigned int &program, const GLchar* name, glm::mat4 value);

void setFloat(unsigned int &program, const GLchar* name, float value);

void setInt(unsigned int &program, const GLchar* name, int value);