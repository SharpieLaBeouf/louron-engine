#include "Camera.h"

#include "../Core/Engine.h"
#include "../Core/Input.h"
#include "../Debug/Assert.h"
#include "../Scene/Entity.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Louron {

	#pragma region SCENE CAMERA

	SceneCamera::SceneCamera()
	{
		m_CameraType = Camera_Type::SceneCamera;
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		L_CORE_ASSERT(width > 0 && height > 0, "Cannot Set Viewport Size to Less or Equal to 0.");
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_Projection = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoBottom = -m_OrthographicSize * 0.5f;
			float orthoTop = m_OrthographicSize * 0.5f;

			m_Projection = glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}
	}

	#pragma endregion

	#pragma region EDITOR CAMERA

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), CameraBase(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		m_CameraType = Camera_Type::EditorCamera;
		UpdateView();
	}

	void EditorCamera::OnUpdate()
	{
		auto& input = Engine::Get().GetInput();

		const glm::vec2& mouse{ input.GetMouseX(), input.GetMouseY() };
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		if(float scroll_delta = input.GetScrollY() * 0.1f; scroll_delta != 0.0f)
			MouseZoom(scroll_delta);

		if (input.GetKey(Key::LeftAlt)) {

			if (input.GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (input.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				MouseRotate(delta);
			else if (input.GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);
		}
		else {

			if (input.GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (input.GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
				MouseRotate(delta);
		}

		UpdateView();
	}

	void EditorCamera::FocusOnEntity(Entity entity)
	{
		glm::vec3 position_to_look_at{};

		// If it is a mesh I'd like to look at the mesh, some meshes have AABBs that are offset from their true transform
		if (entity.HasComponent<MeshFilterComponent>() && entity.HasComponent<MeshRendererComponent>()) 
		{
			position_to_look_at = entity.GetComponent<MeshFilterComponent>().TransformedAABB.Center();
		}
		else 
		{
			position_to_look_at = entity.GetComponent<TransformComponent>().GetGlobalPosition();
		}

		SetFocalPoint(position_to_look_at);
		m_Distance = glm::length(m_Position - position_to_look_at) * 0.75f; // Adjust distance
		UpdateView();
	}

	void EditorCamera::SetFocalPoint(const glm::vec3& focal_point)
	{
		m_FocalPoint = focal_point;
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 5.0f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 5.0f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	#pragma endregion

}