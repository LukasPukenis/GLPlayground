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
#include "Camera.h"

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {

	std::cout << "---------------------opengl-callback-start------------" << std::endl;
	std::cout << "message: " << message << std::endl;
	std::cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "OTHER";
		break;
	}
	std::cout << std::endl;

	std::cout << "id: " << id << std::endl;
	std::cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "HIGH";
		break;
	}
	std::cout << std::endl;
	std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

const GLuint WIDTH = 800, HEIGHT = 600;
GLFWwindow* window;
Camera camera;
double deltaTime, lastFrameTime;
int keysPressed[0xFF];

void setup() {
	camera.setScreenDimensions(WIDTH, HEIGHT);
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WIDTH, HEIGHT, "BSP Map parser", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);
}

void processInputs() {
	if (keysPressed[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (keysPressed[GLFW_KEY_S]) {
		camera.move(Camera::Forward);
	}

	if (keysPressed[GLFW_KEY_W]) {
		camera.move(Camera::Backward);
	}

	if (keysPressed[GLFW_KEY_A]) {
		camera.move(Camera::Left);
	}

	if (keysPressed[GLFW_KEY_D]) {
		camera.move(Camera::Right);
	}
}

void run() {
	// Build and compile our shader program
	Shader ourShader("basic.vert", "basic.frag");

	BSP bsp;
	auto result = bsp.parse("map/maps/q3dm2.bsp");
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
			auto index = vertex + meshVertexes[i + offset];
			vertexes[index].textureIndex = (*polygon)->texture;
			indexes.push_back(index);
		}

		total += length;
		elements.push_back(offset);
	}

	/////////////////////////////////////
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	
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
	glVertexAttribPointer(1, 4, GL_BYTE, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)((10 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(1);
	
	// Texel attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BSP_vertex), (GLvoid*)((3 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(2);

	// Texture array depth/id attribute
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(BSP_vertex), (GLvoid*)((4 * sizeof(GLbyte)) + (10 * sizeof(GL_FLOAT))));
	glEnableVertexAttribArray(3);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(0); // Unbind VAO

	glm::mat4 projection = camera.getProjection();
	camera.setPosition(glm::vec3(0.0, 0.0, 40.0));

	while (!glfwWindowShouldClose(window))
	{		
		deltaTime = glfwGetTime() - lastFrameTime;
		lastFrameTime = glfwGetTime();

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		processInputs();

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
		camera.setDeltaTime(deltaTime);
		view = camera.getView();

		// Get their uniform location
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
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	keysPressed[key] = action;
}

bool firstMouse = true;
double lastMouseX, lastMouseY;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}

	auto sensitivity = 0.5;

	auto diffX = xpos - lastMouseX;
	auto diffY = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;

	diffX *= sensitivity;
	diffY *= sensitivity;

	std::cout << diffY << std::endl;
	camera.adjustPitch(diffY);
	camera.adjustYaw(diffX);
}

int main()
{
	setup();
	run();

	return 0;
}