#pragma once
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

class Render {
public:
	void addVertexes(std::vector<GLfloat> & vertexes);
private:
	std::vector<GLfloat> vertexes;
	std::vector<Shader> shaders;
};