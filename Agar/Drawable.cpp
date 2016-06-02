#include "Drawable.h"

glm::vec3 Drawable::getPosition() const {
	return position;
}

glm::vec3 Drawable::getScale() const {
	return scale;
}

void Drawable::setPosition(const glm::vec3 & pos) {
	position = pos;
}

void Drawable::setScale(const glm::vec3 & scl) {
	scale = scl;
}

void Drawable::setColor(const glm::vec3 & clr) {
	color = clr;
}

glm::vec3 Drawable::getColor() const {
	return color;
}