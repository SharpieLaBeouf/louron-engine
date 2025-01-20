using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Louron
{
    public class Entity
    {

        public readonly uint ID;

        #region Entity Properties

        public string tag
        {
            get
            {
                return EngineCallbacks.TagComponent_GetTag(ID);
            }
            set
            {
                EngineCallbacks.TagComponent_SetTag(ID, value);
            }
        }

        public Transform transform
        {
            get // Create a new reference for a transform, pass it to C++ and store the returned values
            {
                _Transform reference = new _Transform();
                EngineCallbacks.TransformComponent_GetTransform(ID, ref reference);

                _transform.position = reference.position;
                _transform.rotation = reference.rotation;
                _transform.scale = reference.scale;

                return _transform;
            }
            set // Create a new temporary transform proxy to pass to C++ to set the transform
            {
                _Transform temp;
                temp.position = value.position;
                temp.rotation = value.rotation;
                temp.scale = value.scale;

                EngineCallbacks.TransformComponent_SetTransform(ID, ref temp);
            }
        }

        public Entity parent
        {
            get
            {
                return new Entity(EngineCallbacks.Entity_GetParent(ID));
            }

            set
            {
                EngineCallbacks.Entity_SetParent(ID, value.ID);
            }
        }

        #endregion

        #region Entity Public Methods

        public static void Destroy(uint entity_id)
        {
            if (entity_id != System.UInt32.MaxValue) { // Is not null
                EngineCallbacks.Entity_DestroyEntity(entity_id);
            }
        }

        public Entity Instantiate(Prefab prefab)
        {
            EngineCallbacks.Entity_Instantiate(ID, ref prefab.Asset_Handle, out uint prefab_clone_entity_id);
            return new Entity(prefab_clone_entity_id);
        }
        public Entity Instantiate(Prefab prefab, Entity parent)
        {
            EngineCallbacks.Entity_Instantiate(ID, ref prefab.Asset_Handle, out uint prefab_clone_entity_id);
            Entity ent = new Entity(prefab_clone_entity_id);
            ent.parent = parent;
            return ent;
        }
        public Entity Instantiate(Prefab prefab, Vector3 position)
        {
            EngineCallbacks.Entity_Instantiate(ID, ref prefab.Asset_Handle, out uint prefab_clone_entity_id);
            Entity ent = new Entity(prefab_clone_entity_id);
            ent.transform.position = position;
            return ent;
        }
        public Entity Instantiate(Prefab prefab, Vector3 position, Vector3 rotation)
        {
            EngineCallbacks.Entity_Instantiate(ID, ref prefab.Asset_Handle, out uint prefab_clone_entity_id);
            Entity ent = new Entity(prefab_clone_entity_id);
            ent.transform.position = position;
            ent.transform.rotation = rotation;
            return ent;
        }
        public Entity Instantiate(Prefab prefab, Vector3 position, Vector3 rotation, Vector3 scale)
        {
            EngineCallbacks.Entity_Instantiate(ID, ref prefab.Asset_Handle, out uint prefab_clone_entity_id);
            Entity ent = new Entity(prefab_clone_entity_id);
            ent.transform.position = position;
            ent.transform.rotation = rotation;
            ent.transform.scale = scale;
            return ent;
        }

        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return new T() { Entity = this };

            Type componentType = typeof(T);
            EngineCallbacks.Entity_AddComponent(ID, componentType);

            T component = new T() { Entity = this };
            return component;
        }

        public void RemoveComponent<T>() where T : Component, new ()
        {
            if (!HasComponent<T>())
                return;

            Type componentType = typeof(T);
            EngineCallbacks.Entity_RemoveComponent(ID, componentType);
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return EngineCallbacks.Entity_HasComponent(ID, componentType);
        }

        public T? GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        #endregion

        #region Entity Internal and Private Variables

        protected Entity()
        {
            ID = 0;
            _transform = new Transform();
        }

        internal Entity(uint id)
        {
            ID = id;
            _transform = new Transform(ID);
        }

        internal Transform _transform;

        internal struct _Transform
        {
            public Vector3 position;
            public Vector3 rotation;
            public Vector3 scale;
        }

        #endregion

    }
}
