#pragma once
#include <chrono>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include "Drawable.h"
#include "Render.h"

class Game {
public:
	Game();
	void run();
	void pause();
	void tick();
	bool isRunning();
	void addElement(Drawable * element);
	Render * getRender();
private:
	void draw();
	void processInput();
	void processActions();
	std::vector<Drawable *> elements;

	std::vector<char> keyboardInput;
	bool running;
	std::chrono::steady_clock::time_point lastTime;
	uint64_t frame;

	std::mutex inputMutex;
	std::thread inputThread;
	std::thread processThread;
	Render * render;
};