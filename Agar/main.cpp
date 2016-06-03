#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Drawable.h"
#include "Shader.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Utils.h"
#include "BSP.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
const GLuint WIDTH = 800, HEIGHT = 600;

auto CAM_INC = 0.62;
auto CAM_Z = 20.0;
auto CAM_X = 0.0;
auto randomF(double low, double up) {
	return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (up - low)));
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "BSP Map parser", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Build and compile our shader program
	Shader ourShader("basic.vert", "basic.frag");	

	BSP bsp;
	auto result = bsp.parse("map/maps/q3ctf2.bsp");
	auto & faces = bsp.getFaces();
	auto & meshVertexes = bsp.getMeshVertexes();

	typedef GLuint element_t;
	std::vector<element_t> elements;

	bsp.debug();

	auto & vertexes = bsp.getVertexes();

	// extract polygons from faces and draw them
	auto total = 0;
	auto polygon = faces.begin();
	auto faceCnt = 0;
	
	std::vector<unsigned int> indexes;

	while (polygon != faces.end()) {
		faceCnt++;
		polygon = std::find_if(polygon + 1, faces.end(), [&](const auto & item) {
			return item->type == 1;
		});

		if (polygon == faces.end()) break;


		int vertex = (*polygon)->vertex;
		int length = (*polygon)->n_meshverts;
		int offset = (*polygon)->meshvert;

		for (auto i = 0; i < length; i++) {
			indexes.push_back(vertex+ meshVertexes[i+offset]);
		}

		total += length;
		elements.push_back(offset);
	}
	
	/////////////////////////////////////

	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indexes.size(), &indexes[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);	
	glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(BSP_vertex), &vertexes[0], GL_STATIC_DRAW);
	
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 4, GL_BYTE, GL_FALSE, sizeof(BSP_vertex), (GLvoid*) ((10*sizeof(GL_FLOAT)) ));
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(0); // Unbind VAO
			
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.Use();

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		GLint angleLoc = glGetUniformLocation(ourShader.Program, "angle");

		glm::mat4 view;
		glm::mat4 projection;

		view = glm::lookAt(glm::vec3(CAM_X, 0.0, CAM_Z), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

		projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.01f, 1000.0f);
		
		// Get their uniform location			
		auto t = glfwGetTime();

		glUniform1f(angleLoc, glfwGetTime());
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));		

		glm::mat4 model = glm::mat4(1.0);	// just identity
			
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);	

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glDrawElements(GL_TRIANGLES, indexes.size(), GL_UNSIGNED_INT, 0);
		
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_S) {
		std::cout << "Zoom out\n";
		CAM_Z += CAM_INC;
	}

	if (key == GLFW_KEY_W) {
		std::cout << "Zoom in: " << CAM_Z << "\n";
		CAM_Z -= CAM_INC;
	}

	if (key == GLFW_KEY_A) {
		CAM_X -= CAM_INC;
	}

	if (key == GLFW_KEY_D) {
		CAM_X += CAM_INC;
	}
}