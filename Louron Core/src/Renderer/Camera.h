#pragma once

#include <utility>
#include <glm/glm.hpp>

namespace Louron {

	enum class Camera_Type {

		None = 0,
		SceneCamera,
		EditorCamera

	};

	class CameraBase
	{

	public:

		CameraBase() = default;
		CameraBase(const glm::mat4& projection) : m_Projection(projection) { }
		CameraBase(const glm::mat4& projection, const glm::mat4& view_matrix) : m_Projection(projection), m_ViewMatrix(view_matrix) { }

		virtual ~CameraBase() = default;

		virtual const glm::mat4& GetProjection() const { return m_Projection; }
		virtual const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		const Camera_Type& GetCameraType() const { return m_CameraType; }

	protected:

		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_Projection = glm::mat4(1.0f);

		Camera_Type m_CameraType = Camera_Type::None;

	};

	class SceneCamera : public CameraBase
	{
	public:

		enum class ProjectionType : uint8_t { Perspective = 0, Orthographic = 1 };

	public:

		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		void SetOrthographic(float size, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

		const glm::mat4& GetProjection() const override { return m_Projection; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }

	private:

		void RecalculateProjection();

	private:

		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_PerspectiveFOV = glm::radians(60.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 0.0f;
	};

	class Entity;

	class EditorCamera : CameraBase {


	public:

		EditorCamera() { m_CameraType = Camera_Type::EditorCamera; }
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate();

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		void FocusOnEntity(Entity entity);
		void SetFocalPoint(const glm::vec3& focal_point);

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

		const glm::mat4& GetProjection() const override { return m_Projection; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }

	private:

		void UpdateProjection();
		void UpdateView();

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:

		float m_FOV = 60.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

	};

}

