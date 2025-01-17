using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Louron.Entity;

namespace Louron
{

    public abstract class Component
    {
        public Entity Entity { get; internal set; }

        #region Component Properties

        public string tag
        {
            get
            {
                return EngineCallbacks.TagComponent_GetTag(Entity.ID);
            }
            set
            {
                EngineCallbacks.TagComponent_SetTag(Entity.ID, value);
            }
        }

        public Transform transform
        {
            get // Create a new reference for a transform, pass it to C++ and store the returned values
            {
                _Transform reference = new _Transform();
                EngineCallbacks.TransformComponent_GetTransform(Entity.ID, ref reference);

                Entity._transform.position = reference.position;
                Entity._transform.rotation = reference.rotation;
                Entity._transform.scale = reference.scale;

                return Entity._transform;
            }
            set // Create a new temporary transform proxy to pass to C++ to set the transform
            {
                _Transform temp;
                temp.position = value.position;
                temp.rotation = value.rotation;
                temp.scale = value.scale;

                EngineCallbacks.TransformComponent_SetTransform(Entity.ID, ref temp);
            }
        }

        #endregion

        #region Component Functions

        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>()) {
                return new T() { Entity = this.Entity };
            }

            Type componentType = typeof(T);
            EngineCallbacks.Entity_AddComponent(Entity.ID, componentType);

            T component = new T() { Entity = this.Entity };
            return component;
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return;

            Type componentType = typeof(T);
            EngineCallbacks.Entity_RemoveComponent(Entity.ID, componentType);
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return EngineCallbacks.Entity_HasComponent(Entity.ID, componentType);
        }

        public T? GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this.Entity };
            return component;
        }

        // TODO: IMPLEMENT
        public T? GetComponentInParent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this.Entity };
            return component;
        }

        // TODO: IMPLEMENT
        public T[]? GetComponentsInParent<T>() where T : Component, new()
        {
            T[] components = new T[0];

            return components;
        }

        // TODO: IMPLEMENT
        public T? GetComponentInChildren<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this.Entity };
            return component;
        }

        // TODO: IMPLEMENT
        public T[]? GetComponentsInChildren<T>() where T : Component, new()
        {
            T[] components = new T[0];

            return components;
        }

        internal virtual void SetEntity(UInt32 entity_uuid)
        {
            Entity = new Entity(entity_uuid);
        }

        #endregion
    }

    public class TransformComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        //public Transform transform
        //{
        //    get
        //    {
        //        EngineCallbacks.TransformComponent_GetTransform(Entity.ID, out Transform result);
        //        return result;
        //    }
        //    set
        //    {
        //        EngineCallbacks.TransformComponent_SetTransform(Entity.ID, ref value);
        //    }
        //}
    }

    public class TagComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public string Tag
        {
            get
            {
                return EngineCallbacks.TagComponent_GetTag(Entity.ID);
            }
            set
            {
                EngineCallbacks.TagComponent_SetTag(Entity.ID, value);
            }
        }
    }

    public class ScriptComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public string scriptName
        {
            get
            {
                EngineCallbacks.ScriptComponent_GetScript(Entity.ID, out string result);
                return result;
            }
            set
            {
                EngineCallbacks.ScriptComponent_SetScript(Entity.ID, ref value);
            }
        }
    }

    public class PointLightComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public bool active
        {
            get { EngineCallbacks.PointLightComponent_GetActive(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.PointLightComponent_SetActive(Entity.ID, ref value); }
        }
        public Vector4 colour
        {
            get { EngineCallbacks.PointLightComponent_GetColour(Entity.ID, out Vector4 result); return result; }
            set { EngineCallbacks.PointLightComponent_SetColour(Entity.ID, ref value); }
        }
        public float radius
        {
            get { EngineCallbacks.PointLightComponent_GetRadius(Entity.ID, out float result); return result; }
            set { EngineCallbacks.PointLightComponent_SetRadius(Entity.ID, ref value); }
        }
        public float intensity
        {
            get { EngineCallbacks.PointLightComponent_GetIntensity(Entity.ID, out float result); return result; }
            set { EngineCallbacks.PointLightComponent_SetIntensity(Entity.ID, ref value); }
        }
        public ShadowTypeFlag shadowflag
        {
            get { EngineCallbacks.PointLightComponent_GetShadowFlag(Entity.ID, out ShadowTypeFlag result); return result; }
            set { EngineCallbacks.PointLightComponent_SetShadowFlag(Entity.ID, ref value); }
        }
    }

    public class SpotLightComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }
        public bool active
        {
            get { EngineCallbacks.SpotLightComponent_GetActive(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.SpotLightComponent_SetActive(Entity.ID, ref value); }
        }
        public Vector4 colour
        {
            get { EngineCallbacks.SpotLightComponent_GetColour(Entity.ID, out Vector4 result); return result; }
            set { EngineCallbacks.SpotLightComponent_SetColour(Entity.ID, ref value); }
        }
        public float range
        {
            get { EngineCallbacks.SpotLightComponent_GetRange(Entity.ID, out float result); return result; }
            set { EngineCallbacks.SpotLightComponent_SetRange(Entity.ID, ref value); }
        }
        public float angle
        {
            get { EngineCallbacks.SpotLightComponent_GetAngle(Entity.ID, out float result); return result; }
            set { EngineCallbacks.SpotLightComponent_SetAngle(Entity.ID, ref value); }
        }
        public float intensity
        {
            get { EngineCallbacks.SpotLightComponent_GetIntensity(Entity.ID, out float result); return result; }
            set { EngineCallbacks.SpotLightComponent_SetIntensity(Entity.ID, ref value); }
        }
        public ShadowTypeFlag shadowflag
        {
            get { EngineCallbacks.SpotLightComponent_GetShadowFlag(Entity.ID, out ShadowTypeFlag result); return result; }
            set { EngineCallbacks.SpotLightComponent_SetShadowFlag(Entity.ID, ref value); }
        }
    }

    public class DirectionalLightComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public bool active
        {
            get { EngineCallbacks.DirectionalLightComponent_GetActive(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.DirectionalLightComponent_SetActive(Entity.ID, ref value); }
        }
        public Vector4 colour
        {
            get { EngineCallbacks.DirectionalLightComponent_GetColour(Entity.ID, out Vector4 result); return result; }
            set { EngineCallbacks.DirectionalLightComponent_SetColour(Entity.ID, ref value); }
        }
        public float intensity
        {
            get { EngineCallbacks.DirectionalLightComponent_GetIntensity(Entity.ID, out float result); return result; }
            set { EngineCallbacks.DirectionalLightComponent_SetIntensity(Entity.ID, ref value); }
        }
        public ShadowTypeFlag shadowFlag
        {
            get { EngineCallbacks.DirectionalLightComponent_GetShadowFlag(Entity.ID, out ShadowTypeFlag result); return result; }
            set { EngineCallbacks.DirectionalLightComponent_SetShadowFlag(Entity.ID, ref value); }
        }
    }

    public class RigidbodyComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public float mass
        {
            get { EngineCallbacks.RigidbodyComponent_GetMass(Entity.ID, out float result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetMass(Entity.ID, ref value); }
        }

        public float drag
        {
            get { EngineCallbacks.RigidbodyComponent_GetDrag(Entity.ID, out float result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetDrag(Entity.ID, ref value); }
        }

        public float angularDrag
        {
            get { EngineCallbacks.RigidbodyComponent_GetAngularDrag(Entity.ID, out float result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetAngularDrag(Entity.ID, ref value); }
        }
        
        public bool automaticCentreOfMass
        {
            get { EngineCallbacks.RigidbodyComponent_GetAutomaticCentreOfMass(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetAutomaticCentreOfMass(Entity.ID, ref value); }
        }

        public bool useGravity
        {
            get { EngineCallbacks.RigidbodyComponent_GetUseGravity(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetUseGravity(Entity.ID, ref value); }
        }

        public bool isKinematic
        {
            get { EngineCallbacks.RigidbodyComponent_GetIsKinematic(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetIsKinematic(Entity.ID, ref value); }
        }

        public BVector3 positionRestraints
        {
            get { EngineCallbacks.RigidbodyComponent_GetPositionConstraint(Entity.ID, out BVector3 result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetPositionConstraint(Entity.ID, ref value); }
        }

        public BVector3 rotationConstraints
        {
            get { EngineCallbacks.RigidbodyComponent_GetRotationConstraint(Entity.ID, out BVector3 result); return result; }
            set { EngineCallbacks.RigidbodyComponent_SetRotationConstraint(Entity.ID, ref value); }
        }

        public void ApplyForce(Vector3 force, ForceMode forceMode = ForceMode.eFORCE)
        {
            EngineCallbacks.RigidbodyComponent_ApplyForce(Entity.ID, ref force, ref forceMode);
        }

        public void ApplyTorque(Vector3 torque)
        {
            EngineCallbacks.RigidbodyComponent_ApplyTorque(Entity.ID, ref torque);
        }
    }

    public class BoxColliderComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public bool isTrigger
        {
            get { EngineCallbacks.BoxColliderComponent_GetIsTrigger(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.BoxColliderComponent_SetIsTrigger(Entity.ID, ref value); }
        }

        public Vector3 centre
        {
            get { EngineCallbacks.BoxColliderComponent_GetCentre(Entity.ID, out Vector3 result); return result; }
            set { EngineCallbacks.BoxColliderComponent_SetCentre(Entity.ID, ref value); }
        }

        public Vector3 size
        {
            get { EngineCallbacks.BoxColliderComponent_GetSize(Entity.ID, out Vector3 result); return result; }
            set { EngineCallbacks.BoxColliderComponent_SetSize(Entity.ID, ref value); }
        }

        public PhysicsMaterial material
        {
            get { EngineCallbacks.BoxColliderComponent_GetMaterial(Entity.ID, out PhysicsMaterial result); return result; }
            set { EngineCallbacks.BoxColliderComponent_SetMaterial(Entity.ID, ref value); }
        }
    }

    public class SphereColliderComponent : Component
    {
        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }

        public bool isTrigger
        {
            get { EngineCallbacks.SphereColliderComponent_GetIsTrigger(Entity.ID, out bool result); return result; }
            set { EngineCallbacks.SphereColliderComponent_SetIsTrigger(Entity.ID, ref value); }
        }

        public Vector3 centre
        {
            get { EngineCallbacks.SphereColliderComponent_GetCentre(Entity.ID, out Vector3 result); return result; }
            set { EngineCallbacks.SphereColliderComponent_SetCentre(Entity.ID, ref value); }
        }

        public float radius
        {
            get { EngineCallbacks.SphereColliderComponent_GetRadius(Entity.ID, out float result); return result; }
            set { EngineCallbacks.SphereColliderComponent_SetRadius(Entity.ID, ref value); }
        }

        public PhysicsMaterial material
        {
            get { EngineCallbacks.SphereColliderComponent_GetMaterial(Entity.ID, out PhysicsMaterial result); return result; }
            set { EngineCallbacks.SphereColliderComponent_SetMaterial(Entity.ID, ref value); }
        }
    }

}
