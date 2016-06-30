#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "BSP.h"
#include <vector>
#include <string>
#include "Camera.h"

class Game {
public:
	void run();
	void setup(const std::string & mapName, GLuint width, GLuint height);		
	void keyCallback(int key, int action);
	void mouseCallback(double xpos, double ypos);
private:
	void prepareFaces();
	void Game::processInputs();
	std::string mapName;
	BSP bsp;
	std::vector<unsigned int> indexes;
	GLFWwindow* window;
	Camera camera;
	double deltaTime, lastFrameTime;
	int keysPressed[0xFF];
	bool firstMouse = true;
	double lastMouseX, lastMouseY;
	GLuint WIDTH, HEIGHT;;
};
