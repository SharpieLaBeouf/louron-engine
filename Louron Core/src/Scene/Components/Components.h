#pragma once

// Louron Core Headers
#include "UUID.h"

// C++ Standard Library Headers
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <optional>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace Louron {

    class Scene;
    class Entity;
    class Camera;

    /// <summary>
    /// Flags that are set to determine what state changes have occured
    /// each frame so the TransformSystem may process these changes.
    /// </summary>
    enum TransformFlags : uint32_t {

        TransformFlag_None              = 0,        

        TransformFlag_PositionUpdated   = 1U << 0,  // Only add this flag where there have been changes made to the entities position
        TransformFlag_RotationUpdated   = 1U << 1,  // Only add this flag where there have been changes made to the entities rotation
        TransformFlag_ScaleUpdated      = 1U << 2,  // Only add this flag where there have been changes made to the entities scale

    };

    struct Component {

        virtual ~Component() = default;

        std::shared_ptr<Entity> entity;

        template<typename T>
        T& GetComponent();

        template<typename T>
        std::vector<T&> GetComponents();
    };

    struct IDComponent : public Component {
        UUID ID;

        IDComponent() = default;
        IDComponent(UUID uuid) : ID(uuid) { }
        IDComponent(const IDComponent&) = default;
    };

	struct TagComponent : public Component {

		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) : Tag(name) { }
	};

    enum CameraClearFlags {
        COLOUR_ONLY = 0,
        SKYBOX = 1
    };

	struct CameraComponent : public Component {

		std::shared_ptr<Camera> Camera = nullptr;
		bool Primary = true;
        CameraClearFlags ClearFlags = CameraClearFlags::COLOUR_ONLY;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

    // TODO: Implement Audio Components

    struct AudioListener : public Component {
        int i = 0;
    };

    struct AudioEmitter : public Component {

    };

	struct Transform : public Component {

	private:
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f);
		glm::vec3 m_Scale = glm::vec3(1.0f);
		glm::mat4 m_Transform = glm::mat4(1.0f);

        std::optional<glm::mat4> m_WorldTransform = std::nullopt;

        TransformFlags m_StateFlags = TransformFlag_None;

    public:

        Transform();
        Transform(const Transform&) = default;
        Transform(const glm::vec3& translation);

        // FLAGS
        void AddFlag(TransformFlags flag);
        void RemoveFlag(TransformFlags flag);
        bool CheckFlag(TransformFlags flag) const;
        bool NoFlagsSet() const;
        void ClearFlags();
        TransformFlags GetFlags() const;

        void SetPosition(const glm::vec3& newPosition);
        void SetPositionX(const float& newXPosition);
        void SetPositionY(const float& newYPosition);
        void SetPositionZ(const float& newZPosition);

        void SetRotation(const glm::vec3& newRotation);
        void SetRotationX(const float& newXRotation);
        void SetRotationY(const float& newYRotation);
        void SetRotationZ(const float& newZRotation);

        void SetScale(const glm::vec3& newScale);
        void SetScaleX(const float& newXScale);
        void SetScaleY(const float& newYScale);
        void SetScaleZ(const float& newZScale);

        void Translate(const glm::vec3& vector);
        void TranslateX(const float& deltaTranslationX);
        void TranslateY(const float& deltaTranslationY);
        void TranslateZ(const float& deltaTranslationZ);

        void Rotate(const glm::vec3& vector);
        void RotateX(const float& deltaRotationX);
        void RotateY(const float& deltaRotationY);
        void RotateZ(const float& deltaRotationZ);

        void Scale(const glm::vec3& vector);
        void ScaleX(const float& deltaScaleX);
        void ScaleY(const float& deltaScaleY);
        void ScaleZ(const float& deltaScaleZ);

        glm::vec3 GetPosition();
        glm::vec3 GetRotation();
        glm::vec3 GetScale();

        const glm::vec3& GetLocalPosition();
        const glm::vec3& GetLocalRotation();
        const glm::vec3& GetLocalScale();

        glm::mat4 GetTransform() const;
        glm::mat4 GetLocalTransform() const;

        void SetTransform(const glm::mat4& transform);

        operator const glm::mat4()&;
        glm::mat4 operator*(const Transform& other) const;

    private:

        friend class Scene;
        friend class SceneSerializer;
        friend class TransformSystem;
        friend class PhysicsSystem;

	};

    template<typename... Component>
    struct ComponentGroup { };

    struct MeshFilter;
    struct MeshRenderer;

    struct PointLightComponent;
    struct SpotLightComponent;
    struct DirectionalLightComponent;
    struct SkyboxComponent;

    struct Rigidbody;
    struct SphereCollider;
    struct BoxCollider;

    using AllComponents = ComponentGroup<

        Component,

        IDComponent,
        TagComponent,

        CameraComponent,

        AudioListener,
        AudioEmitter,

        Transform, 

        MeshFilter, 
        MeshRenderer,

        PointLightComponent, 
        SpotLightComponent, 
        DirectionalLightComponent, 
        SkyboxComponent, 

        Rigidbody,
        SphereCollider,
        BoxCollider
    
    >;

}