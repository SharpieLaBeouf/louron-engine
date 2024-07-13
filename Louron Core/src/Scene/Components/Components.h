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
#include <glm/gtc/quaternion.hpp>

namespace Louron {

    class Scene;
    class Entity;
    class Camera;

    /// <summary>
    /// Flags that are set to determine what state changes have occured
    /// each frame so the TransformSystem may process these changes.
    /// </summary>
    enum TransformFlags : uint8_t {

        TransformFlag_None                      = 0,        

        TransformFlag_PropertiesUpdated         = 1U << 0,  // Only add this flag where there have been changes made to the transform proprties
        TransformFlag_GlobalTransformUpdated    = 1U << 1,  // Only add this flag where there have been changes made to a parent in the hierarchy
    };

    struct Component {

        virtual ~Component() = default;

        std::shared_ptr<Entity> entity;

        template<typename T>
        T* GetComponent();

        template<typename T>
        T* GetComponentInParent();

        template<typename T>
        std::vector<T*> GetComponentsInParent();

        template<typename T>
        T* GetComponentInChildren();

        template<typename T>
        std::vector<T*> GetComponentsInChildren();

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

    struct HierarchyComponent : public Component {

    public:

        HierarchyComponent() = default;
        HierarchyComponent(const HierarchyComponent&) = default;

        void AttachParent(const UUID& newParentID);
        void DetachParent();

        void DetachChildren();
        void RehomeChildren(const UUID& newParentID);

        Entity FindChild(const UUID& childUUID) const;
        Entity FindChild(const std::string& childName) const;
        const std::vector<UUID>& GetChildren() const;

        Entity GetParentEntity() const;
        const UUID& GetParentID() const;
        bool HasParent() const;

    private:

        UUID m_Parent = NULL_UUID;
        std::vector<UUID> m_Children;

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

    };

    struct AudioEmitter : public Component {

    };

	struct Transform : public Component {

	private:
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f);
		glm::vec3 m_Scale = glm::vec3(1.0f);

		glm::mat4 m_LocalTransform = glm::mat4(1.0f);
        glm::mat4 m_GlobalTransform = glm::mat4(1.0f);

        TransformFlags m_StateFlags = TransformFlag_None;

        void OnTransformUpdated();
        void UpdateLocalTransformMatrix();

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
        void SetGlobalPosition(const glm::vec3& newPosition);
        void SetPositionX(const float& newXPosition);
        void SetPositionY(const float& newYPosition);
        void SetPositionZ(const float& newZPosition);

        void SetRotation(const glm::vec3& newRotation);
        void SetGlobalRotation(const glm::vec3& newRotation);
        void SetRotationX(const float& newXRotation);
        void SetRotationY(const float& newYRotation);
        void SetRotationZ(const float& newZRotation);

        void SetScale(const glm::vec3& newScale);
        void SetGlobalScale(const glm::vec3& newScale);
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

        glm::vec3 GetGlobalPosition();
        glm::vec3 GetGlobalRotation();
        glm::vec3 GetGlobalScale();

        const glm::vec3& GetLocalPosition() const;
        const glm::vec3& GetLocalRotation() const;
        const glm::vec3& GetLocalScale() const;

        const glm::mat4& GetGlobalTransform();
        const glm::mat4& GetLocalTransform();

        void SetTransform(const glm::mat4& transform);

        operator const glm::mat4()&;
        glm::mat4 operator*(const Transform& other) const;

    private:

        friend class Scene;
        friend class SceneSerializer;
        friend class TransformSystem;
        friend class PhysicsSystem;

        friend struct HierarchyComponent;

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
        HierarchyComponent,

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