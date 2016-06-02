#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Drawable {
public:
	void setPosition(const glm::vec3 & pos);
	void setScale(const glm::vec3 & scl);
	void setColor(const glm::vec3 & clr);

	glm::vec3 getPosition() const;
	glm::vec3 getScale() const;
	glm::vec3 getColor() const;
private:
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 color;
};