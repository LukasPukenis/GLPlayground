#include "Game.h"
#include <chrono>
#include <iostream>
#include <conio.h>
#include <string>
#include <mutex>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Render.h"

const int THREAD_DELAY = 16;	//60fps
const GLuint WIDTH = 800, HEIGHT = 600;

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void Game::addElement(Drawable * element) {
	elements.push_back(element);
}

void Game::processInput() {
	auto c = _getch();
	if (c == 224) return;	//ignore special indicator from keyboard

	inputMutex.lock();
	keyboardInput.push_back(c);
	inputMutex.unlock();
	std::cout << "[" << c << "]" << std::endl;
}

void Game::processActions() {
	if (keyboardInput.size() > 0) {
		inputMutex.lock();
		auto c = keyboardInput.back();
		keyboardInput.pop_back();
		inputMutex.unlock();
		std::cout << "Buffer contained: " << c << " buffer size: " << keyboardInput.size() << std::endl;
	}
}

// main game handler, this does all the tasks
void Game::draw() {
	// todo
}

Game::Game() :
	lastTime(std::chrono::high_resolution_clock::now()),
	frame(0)
{
	render = new Render();

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);
};

void Game::tick() {
	auto newTime = std::chrono::high_resolution_clock::now();
	uint64_t timeDiff;

	timeDiff = std::chrono::duration_cast<std::chrono::nanoseconds>(newTime - lastTime).count();

	if (timeDiff >= 166666660) {
		lastTime = newTime;
		frame++;
		draw();
	}
}

void Game::run() {
	lastTime = std::chrono::high_resolution_clock::now();
	running = true;

	inputThread = std::thread([this]() {
		while (running) {
			this->processInput();
			//std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_DELAY));			std::cout so why sleep
		}
	});

	processThread = std::thread([this]() {
		while (running) {
			this->processActions();
			std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_DELAY));
		}
	});

	while (running) tick();
	inputThread.join();
	processThread.join();
}

void Game::pause() {
	running = false;
}

bool Game::isRunning() {
	return running;
}

Render * Game::getRender() {
	return render;
}