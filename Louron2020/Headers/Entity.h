#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


class Transform {
	
public:

	glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

public:

	Transform() = default;
	Transform(const Transform&) = default;
	Transform(const glm::vec3 & translation) : Translation(translation) {}

	void translate(glm::vec3 vector) { Translation += vector; }
	void rotate(glm::vec3 vector) { Rotation += vector; }
	void scale(glm::vec3 vector) { Scale += vector; }

	glm::mat4 getTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

		return glm::translate(glm::mat4(1.0f), Translation)
			* rotation
			* glm::scale(glm::mat4(1.0f), Scale);
	}
};


class Entity {



};