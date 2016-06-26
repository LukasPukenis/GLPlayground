#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cassert>

Camera::Camera():screenWidth(0), screenHeight(0), yaw(-90), moveSpeed(1000.0f) {	
}

const glm::mat4 Camera::getView() {
	return view;
}

const glm::mat4 Camera::getProjection() {
	return projection;
}

#include <iostream>

void Camera::setPosition(const glm::vec3 & pos) {
	position = pos;
	updateView();
}

const glm::vec3 Camera::getPosition() {
	return position;
}

void Camera::updateView() {
	if (pitch > 89.9) pitch = 89.9;
	if (pitch < -89.9) pitch = -89.9;

	FORWARD.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	FORWARD.y = sin(glm::radians(pitch));	
	FORWARD.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

	view = glm::lookAt(
		position,
		position + Camera::FORWARD,
		glm::vec3(0.0, 1.0, 0.0)
	);
}

void const Camera::setScreenDimensions(unsigned int width, unsigned int height) {
	screenWidth = width;
	screenHeight = height;
	updateProjection();
}

void Camera::updateProjection() {
	assert(screenWidth > 0 || screenHeight > 0, "Screen dimensions for camera must be initialized in order to form perspective");

	projection = glm::perspective(45.0f, (GLfloat)screenWidth / (GLfloat)screenHeight, 10.1f, 1500.0f);
}

void Camera::adjustPitch(double _pitch) {
	pitch += _pitch;
	updateView();
}

void Camera::adjustYaw(double _yaw) {
	yaw += _yaw;
	updateView();
}

void Camera::setDeltaTime(float _deltaTime) {
	deltaTime = _deltaTime;
}

void Camera::move(Direction direction) {
	auto speed = moveSpeed * deltaTime;

	switch (direction) {
		case Camera::Forward:
			setPosition(getPosition() - (Camera::FORWARD * speed));
			break;
		case Camera::Backward:
			setPosition(getPosition() + (Camera::FORWARD * speed));
			break;
		case Camera::Left:
			setPosition(getPosition() - (glm::normalize(glm::cross(Camera::FORWARD, Camera::UP)) * speed));
			break;
		case Camera::Right:
			setPosition(getPosition() + (glm::normalize(glm::cross(Camera::FORWARD, Camera::UP)) * speed));
			break;
	}
}