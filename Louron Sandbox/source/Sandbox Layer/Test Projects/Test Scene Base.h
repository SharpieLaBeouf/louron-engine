#pragma once

// For scenes that use old light structs that have been removed from core.
struct ManualLight {

	glm::vec3 position = glm::vec3(0.0f);

	glm::vec4 ambient = glm::vec4(1.0f);
	glm::vec4 diffuse = glm::vec4(1.0f);
	glm::vec4 specular = glm::vec4(1.0f);

	ManualLight() = default;
};

class TestScene {

public:
	TestScene() = default;
	virtual ~TestScene() = default;

	virtual void OnAttach() { }
	virtual void OnDetach() { }

	virtual void Update() { }
	virtual void UpdateGUI() { }

private:
	virtual void Draw() { }

};