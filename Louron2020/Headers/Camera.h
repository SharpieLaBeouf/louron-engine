#pragma once

#include "../Headers/Input.h"
#include "../Headers/Window.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Camera {

public:
	
	Camera() = default;
	Camera(Window* wnd, glm::vec3 position = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f) {
		m_Window = wnd;
		m_Input = m_Window->getInput();
		m_CameraPos = position;
		m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		m_WorldUp = up;
		m_Yaw = yaw;
		m_Pitch = pitch;

		UpdateCameraVectors();
	}
	~Camera() { }

	glm::mat4 getViewMatrix() { return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp); }
	glm::vec3 getPosition() { return m_CameraPos; }

	void Update(float deltaTime) {
		processKeyboard(deltaTime);

		if (MouseToggledOff) processMouse(); else if (!MouseToggledOff) m_FirstMouse = true;
	}

	void processKeyboard(float deltaTime) {
		float modifier = (m_Input->GetKey(GLFW_KEY_LEFT_CONTROL)) ? 1.5f : 1.0f;
		float velocity = modifier * MovementSpeed * deltaTime;

		if (m_Input->GetKey(GLFW_KEY_W))
			m_CameraPos += velocity * m_CameraFront;
		if (m_Input->GetKey(GLFW_KEY_S))
			m_CameraPos -= velocity * m_CameraFront;

		if (m_Input->GetKey(GLFW_KEY_SPACE))
			m_CameraPos += velocity * m_WorldUp * MovementYDamp;
		if (m_Input->GetKey(GLFW_KEY_LEFT_SHIFT))
			m_CameraPos -= velocity * m_WorldUp * MovementYDamp;

		if (m_Input->GetKey(GLFW_KEY_D))
			m_CameraPos += velocity * m_CameraRight;
		if (m_Input->GetKey(GLFW_KEY_A))
			m_CameraPos -= velocity * m_CameraRight;
	}

	void processMouse(bool constrainPitch = true) {
		float xpos = m_Input->GetMouseX(), ypos = m_Input->GetMouseY();

		if (m_FirstMouse) {
			m_LastMouseX = xpos;
			m_LastMouseY = ypos;
			m_FirstMouse = false;
		}

		float xoffset = xpos - m_LastMouseX;
		float yoffset = m_LastMouseY - ypos;

		m_LastMouseX = xpos;
		m_LastMouseY = ypos;

		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		if (constrainPitch) {
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		UpdateCameraVectors();
	}
	
	float FOV = 60.0f;
	float MovementSpeed = 10.0f;
	float MovementYDamp = 0.65f;
	float MouseSensitivity = 0.1f;
	bool MouseToggledOff = true;

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

	void UpdateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_CameraFront = glm::normalize(front);

		m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
		m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
	}
};