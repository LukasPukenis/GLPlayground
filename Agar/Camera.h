#pragma once

#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera {
public:
	enum Direction {Forward, Backward, Left, Right, PITCH_UP, PITCH_DOWN, YAW_DOWN, YAW_UP};
	glm::vec3 UP = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 FORWARD = glm::vec3(0.0, 0.0, -1.0);

	Camera();
	void setPosition(const glm::vec3 & pos);
	const glm::vec3 getPosition();

	void const setScreenDimensions(unsigned int width, unsigned int height);
	glm::mat4 const getView();
	glm::mat4 const getProjection();
	void move(Direction direction);
	void adjustPitch(double pitch);
	void adjustYaw(double yaw);

	// Camera uses deltaTime to calculate the movement speed
	void setDeltaTime(float deltaTime);
private:
	void updateView();
	void updateProjection();

	unsigned int screenWidth;
	unsigned int screenHeight;

	double pitch, yaw, roll;
	float deltaTime;
	float moveSpeed;
	glm::vec3 position;
	glm::mat4 view;
	glm::mat4 projection;
};