#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


struct TransformComponent {
	
public:

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

public:

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3 & translation) : position(translation) {}

	operator glm::mat4 () { return this->getTransform(); }

	void Translate(glm::vec3 vector) { position += vector; }
	void Rotate(glm::vec3 vector) { rotation += vector; }
	void Scale(glm::vec3 vector) { scale += vector; }

	glm::mat4 getTransform() const
	{
		return glm::translate(glm::mat4(1.0f), position)
			* glm::toMat4(glm::quat(glm::radians(rotation)))
			* glm::scale(glm::mat4(1.0f), scale);
	}
};


class Entity {

public:


private:


};