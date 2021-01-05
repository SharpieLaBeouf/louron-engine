#pragma once
#include <vector>

#include "../Headers/Input.h"
#include "../Headers/Window.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {

public:

	// Camera Options

	float FOV = 60.0f;
	float MovementSpeed = 10.0f;
	float MovementYDamp = 0.65f;
	float MouseSensitivity = 0.1f;
	bool MouseToggledOff = true;

	Camera() = default;
	Camera(Window* wnd, glm::vec3 position = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
	~Camera() { }

	glm::mat4 getViewMatrix() { return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp); }
	glm::vec3 getPosition() { return m_CameraPos; }

	void Update(float deltaTime);

	void processKeyboard(float deltaTime);
	void processMouse(bool constrainPitch = true);
	
private:

	Window* m_Window;
	InputManager* m_Input;

	glm::vec3 m_CameraPos;
	glm::vec3 m_CameraUp;
	glm::vec3 m_CameraRight;
	glm::vec3 m_CameraFront;
	glm::vec3 m_WorldUp;

	float m_Yaw;
	float m_Pitch;
	float m_LastMouseX = 0;
	float m_LastMouseY = 0;
	
	bool m_FirstMouse = true;

	void UpdateCameraVectors();
};