#pragma once

#include "Abstracted GL/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

//Components
class Transform;

//Base Entity Class
class Entity {

public:
	Entity() = default;

private:

};

class Transform : public Entity {

public:

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	Transform() = default;
	Transform(const Transform&) = default;

	void Translate(glm::vec3 vector) { position += vector; }
	void Rotate(glm::vec3 vector) { rotation += vector; }
	void Scale(glm::vec3 vector) { scale += vector; }

	operator glm::mat4() { return this->getTransform(); }
	glm::mat4 getTransform() const {
		return glm::translate(glm::mat4(1.0f), position)
			* glm::toMat4(glm::quat(glm::radians(rotation)))
			* glm::scale(glm::mat4(1.0f), scale);
	}
};


